#include "meshloader.hpp"

#include "fastgltf/core.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/types.hpp"
#include "samplermanager.hpp"
#include "texturemanager.hpp"
#include "vulkanengine.hpp"

#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <spdlog/spdlog.h>

namespace selwonk::vulkan {

glm::vec4 GltfMesh::convertVector(const fastgltf::math::nvec4& vec) {
  return glm::vec4(vec[0], vec[1], vec[2], vec[3]);
}

fastgltf::Asset MeshLoader::loadAsset(core::Vfs::FilePtr file) {
  SPDLOG_INFO("Loading gltf {}", file->c_str());

  std::vector<char> buffer;
  file->readfull(buffer);

  auto data = fastgltf::GltfDataBuffer::FromBytes((std::byte*)buffer.data(),
                                                  buffer.size());
  if (data.error() != fastgltf::Error::None) {
    throw LoadException(data.error());
  }

  fastgltf::Options options = fastgltf::Options::DontRequireValidAssetMember |
                              fastgltf::Options::AllowDouble;
  fastgltf::Parser parser;
  auto load = parser.loadGltf(data.get(), "/", options);
  if (load.error() != fastgltf::Error::None) {
    throw LoadException(load.error());
  }

  return std::move(load.get());
}

GltfMesh::~GltfMesh() {
  auto& meshMap = VulkanEngine::get().mMeshes;
  for (auto& root : mRootNodes) {
    root.second->walk([&](auto& node) {
      if (node.mMesh.valid()) {
        meshMap.decRef(node.mMesh);
      }
    });
  }
}

GltfMesh::GltfMesh(const fastgltf::Asset& asset) {
  auto& engine = VulkanEngine::get();
  auto& interop = assets::INativeHandleProvider::get();

  std::vector<SamplerManager::Handle> samplers;
  for (auto& sampler : asset.samplers) {
    assets::SamplerConfig key{
        .mMinFilter = sampler.minFilter.value_or(fastgltf::Filter::Nearest),
        .mMagFilter = sampler.magFilter.value_or(fastgltf::Filter::Nearest),
        // TODO: Import wrapping mode
    };
    samplers.push_back(interop.getSampler(key));
  }

  std::vector<TextureManager::Handle> images;
  for (auto& img : asset.images) {
    images.push_back(interop.loadTextureAsync(img.name.c_str(), asset, img.data));
  }

  std::vector<assets::Material> materials;
  for (auto& mat : asset.materials) {
    assets::Material newMat;
    glm::vec4 matFactors;
    matFactors.x = mat.pbrData.metallicFactor;
    matFactors.y = mat.pbrData.roughnessFactor;

    auto data = interop.addMaterial({
        .colorFactors = convertVector(mat.pbrData.baseColorFactor),
        .metalRoughnessFactors = matFactors,
    });

    newMat.mDataIndex = data;
    newMat.mPass = mat.alphaMode == fastgltf::AlphaMode::Blend
                       ? assets::Material::Pass::Translucent
                       : assets::Material::Pass::Opaque;

    if (mat.pbrData.baseColorTexture.has_value()) {
      size_t img =
          asset.textures[mat.pbrData.baseColorTexture.value().textureIndex]
              .imageIndex.value();
      newMat.mTexture = images[img];
      size_t samplerIdx =
          asset.textures[mat.pbrData.baseColorTexture.value().textureIndex]
              .samplerIndex.value();
      newMat.mSampler = samplers[samplerIdx];
    } else {
      // Vertex colors only
      newMat.mTexture = interop.getWhite();
      newMat.mSampler = interop.getSampler({
        .mMinFilter = fastgltf::Filter::Nearest,
        .mMagFilter = fastgltf::Filter::Nearest,
      });
    }
    materials.push_back(newMat);
  }

  std::vector<assets::MeshData::Handle> meshes;
  for (auto& mesh : asset.meshes) {
    meshes.push_back(Mesh::load(asset, mesh, materials));
  }

  // Use three passes: First to convert nodes to our format, then to build the
  // hierarchy. Finally determine root nodes.
  std::vector<std::shared_ptr<Node>> nodes;
  for (auto node : asset.nodes) {
    auto newNode = std::make_shared<Node>();
    nodes.push_back(newNode);

    if (node.meshIndex.has_value()) {
      newNode->mMesh = meshes[node.meshIndex.value()];
      engine.mMeshes.incRef(newNode->mMesh);
    }

    std::visit(
        fastgltf::visitor{
            [&](fastgltf::math::fmat4x4 matrix) {
              assert(false && "NOPE!");
              // memcpy(&newNode->mLocalTransform,
              // matrix.data(), sizeof(matrix));
            },
            [&](fastgltf::TRS transform) {
              glm::vec3 tl(transform.translation[0], transform.translation[1],
                           transform.translation[2]);
              glm::quat rot(transform.rotation[3], transform.rotation[0],
                            transform.rotation[1], transform.rotation[2]);
              glm::vec3 sc(transform.scale[0], transform.scale[1],
                           transform.scale[2]);

              newNode->mLocalTransform = {.mTranslation = glm::vec4(tl, 1.f),
                                          .mRotation = rot,
                                          .mScale = sc};
            }},
        node.transform);
    newNode->mName = node.name;
  }

  for (int i = 0; i < asset.nodes.size(); i++) {
    auto& node = asset.nodes[i];
    auto& sceneNode = nodes[i];
    for (auto& child : node.children) {
      sceneNode->mChildren.push_back(nodes[child]);
      nodes[child]->mParent = sceneNode.get();
    }
  }

  for (int i = 0; i < asset.nodes.size(); i++) {
    auto& node = asset.nodes[i];
    auto& sceneNode = nodes[i];
    if (sceneNode->mParent == nullptr) {
      mRootNodes[node.name.c_str()] = sceneNode;
    }
  }

  // Materials, meshes, and textures have had their ref counts incremented by
  // nodes Free our copies

  // TODO: Retain names during load to log unused assets
  for (auto& mat : materials) {
    if (interop.decRef(mat.mDataIndex)) {
      SPDLOG_WARN("Unused material in GLTF");
    }
  }
  for (auto& mesh : meshes) {
    if (engine.mMeshes.decRef(mesh)) {
      SPDLOG_WARN("Unused mesh in GLTF");
    }
  }
  for (auto& tex : images) {
    if (interop.decRef(tex)) {
      SPDLOG_WARN("Unused texture in GLTF");
    }
  }

  // TODO: Do this in engine, currently jobs don't own their asset
  engine.getThreadPool().awaitAll();
  engine.getThreadPool().finalise();
}

void GltfMesh::Node::instantiate(ecs::Registry& ecs,
                                 const ecs::Transform& transform) {
  auto entity = ecs.createEntity();
  auto localModelMat = transform.apply(mLocalTransform);

  ecs.addComponent<ecs::Transform>(entity, {localModelMat});
  if (mMesh.valid()) {
    ecs.addComponent<ecs::Renderable>(entity, {
                                                  .mMesh = mMesh,
                                              });
  }

  if (!mName.empty()) {
    ecs.addComponent<ecs::Named>(entity, {mName});
  }

  for (auto& child : mChildren) {
    child->instantiate(ecs, {localModelMat});
  }
}

void GltfMesh::instantiate(ecs::Registry& ecs,
                           const ecs::Transform& transform) {
  for (auto& root : mRootNodes) {
    root.second->instantiate(ecs, transform);
  }
}

std::unique_ptr<GltfMesh> MeshLoader::loadGltf(core::Vfs::FilePtr file) {
  auto asset = loadAsset(std::move(file));

  return std::make_unique<GltfMesh>(asset);
}

} // namespace selwonk::vulkan

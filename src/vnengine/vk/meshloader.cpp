#include "meshloader.hpp"

#include "fastgltf/core.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/types.hpp"
#include "material.hpp"
#include "texturemanager.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanengine.hpp"
#include <vncore/bumpallocator.hpp>

#include <fmt/base.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <memory>

namespace selwonk::vulkan {

vk::Filter
GltfMesh::convertFilter(fastgltf::Optional<fastgltf::Filter> filter) {
  using enum fastgltf::Filter;
  switch (filter.value_or(Nearest)) {
  case Nearest:
  case NearestMipMapLinear:
  case NearestMipMapNearest:
    return vk::Filter::eNearest;
  case Linear:
  case LinearMipMapLinear:
  case LinearMipMapNearest:
    return vk::Filter::eLinear;
  }
  std::unreachable();
}

vk::SamplerMipmapMode
GltfMesh::convertMipmapMode(fastgltf::Optional<fastgltf::Filter> mode) {
  using enum fastgltf::Filter;
  switch (mode.value_or(Nearest)) {
  case Nearest:
  case NearestMipMapLinear:
  case NearestMipMapNearest:
    return vk::SamplerMipmapMode::eNearest;
  case Linear:
  case LinearMipMapLinear:
  case LinearMipMapNearest:
    return vk::SamplerMipmapMode::eLinear;
  }
  std::unreachable();
}

glm::vec4 GltfMesh::convertVector(const fastgltf::math::nvec4& vec) {
  return glm::vec4(vec[0], vec[1], vec[2], vec[3]);
}

fastgltf::Asset MeshLoader::loadAsset(core::Vfs::FilePtr file) {
  fmt::println("Loading gltf {}", file->c_str());

  std::vector<std::byte> buffer;
  file->readfull(buffer);

  auto data = fastgltf::GltfDataBuffer::FromBytes(buffer.data(), buffer.size());
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
  for (auto& mesh : mMeshes) {
    meshMap.decRef(mesh.second);
  }

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

  std::vector<SamplerManager::Handle> samplers;
  for (auto& sampler : asset.samplers) {
    vk::SamplerCreateInfo info = {
        .magFilter = convertFilter(sampler.magFilter),
        .minFilter = convertFilter(sampler.minFilter),
        .mipmapMode = convertMipmapMode(sampler.minFilter),
        .minLod = 0,
        .maxLod = vk::LodClampNone,
    };
    samplers.push_back(engine.getSamplers().get(info));
  }

  std::vector<TextureManager::Handle> images;
  for (auto& img : asset.images) {
    auto& textures = engine.getTextureManager();
    try {
      auto image = Image::load(asset, img);
      images.push_back(textures.insert(image));
    } catch (std::runtime_error e) {
      fmt::println("Failed to load image {}", img.name);
      images.push_back(engine.getTextureManager().getMissing());
    }
  }

  std::vector<Material> materials;
  for (auto& mat : asset.materials) {
    Material newMat;
    glm::vec4 metFactors;
    metFactors.x = mat.pbrData.metallicFactor;
    metFactors.y = mat.pbrData.roughnessFactor;

    auto data = engine.mMaterials.insert({
        .colorFactors = convertVector(mat.pbrData.baseColorFactor),
        .metalRoughnessFactors = metFactors,
    });

    newMat.mDataIndex = data;
    newMat.mPass = mat.alphaMode == fastgltf::AlphaMode::Blend
                       ? Material::Pass::Translucent
                       : Material::Pass::Opaque;

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
      newMat.mTexture = engine.getTextureManager().getWhite();
      newMat.mSampler = engine.mDefaultMaterial.mSampler;
    }
    materials.push_back(newMat);
  }

  for (auto& mesh : asset.meshes) {
    mMeshes[mesh.name.c_str()] = Mesh::load(asset, mesh, materials);
  }

  // Use three passes: First to convert nodes to our format, then to build the
  // hierarchy. Finally determine root nodes.
  std::vector<std::shared_ptr<Node>> nodes;
  for (auto node : asset.nodes) {
    auto newNode = std::make_shared<Node>();
    nodes.push_back(newNode);

    if (node.meshIndex.has_value()) {
      newNode->mMesh =
          mMeshes[asset.meshes[node.meshIndex.value()].name.c_str()];
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

  // Materials and textures have had their ref counts incremented by meshes
  for (auto& mat : materials) {
    engine.mMaterials.decRef(mat.mDataIndex);
  }
  for (auto& tex : images) {
    engine.getTextureManager().decRef(tex);
  }
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

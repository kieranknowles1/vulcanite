#include "bumpallocator.hpp"
#include "fastgltf/core.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/types.hpp"
#include "material.hpp"
#include "meshloader.hpp"
#include "shader.hpp"
#include "utility.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanengine.hpp"
#include "vulkanhandle.hpp"

#include <fmt/base.h>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

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
}

glm::vec4 GltfMesh::convertVector(const fastgltf::math::nvec4& vec) {
  return glm::vec4(vec[0], vec[1], vec[2], vec[3]);
}

fastgltf::Asset MeshLoader::loadAsset(Vfs::SubdirPath path) {
  auto& vfs = VulkanEngine::get().getVfs();
  fmt::println("Loading gltf {}", path.string());

  std::vector<std::byte> buffer;
  vfs.readfull(Vfs::Meshes / path, buffer);

  auto data = fastgltf::GltfDataBuffer::FromBytes(buffer.data(), buffer.size());
  if (data.error() != fastgltf::Error::None) {
    throw LoadException(data.error());
  }

  fastgltf::Options options = fastgltf::Options::DontRequireValidAssetMember |
                              fastgltf::Options::AllowDouble |
                              fastgltf::Options::LoadExternalBuffers;
  fastgltf::Parser parser;
  auto load = parser.loadGltf(data.get(), "/", options);
  if (load.error() != fastgltf::Error::None) {
    throw LoadException(load.error());
  }

  return std::move(load.get());
}

GltfMesh::GltfMesh(const fastgltf::Asset& asset)
    : mMaterialBuffer(sizeof(interop::MaterialData) * asset.materials.size(),
                      vk::BufferUsageFlagBits::eUniformBuffer) {
  auto& handle = VulkanHandle::get();
  auto& engine = VulkanEngine::get();

  for (auto& sampler : asset.samplers) {
    fmt::println("Sampler {}", sampler.name);
    vk::SamplerCreateInfo info = {
        .magFilter = convertFilter(sampler.magFilter),
        .minFilter = convertFilter(sampler.minFilter),
        .mipmapMode = convertMipmapMode(sampler.minFilter),
        .minLod = 0,
        .maxLod = vk::LodClampNone,
    };
    vk::Sampler sampl;
    check(handle.mDevice.createSampler(&info, nullptr, &sampl));
    mSamplers.push_back(sampl);
  }

  std::vector<std::shared_ptr<Image>> images;
  for (auto& _ : asset.images) {
    // TODO: Texture loading
    images.push_back(VulkanEngine::get().getErrorTexture());
  }

  std::array<DescriptorAllocator::PoolSizeRatio, 1> poolSizes = {
      {{vk::DescriptorType::eCombinedImageSampler, 1.0f}},
  };
  mDescriptorAllocator.init(asset.materials.size(), poolSizes);
  for (auto& mat : asset.materials) {
    fmt::println("Material {}", mat.name);
    auto newMat = std::make_shared<Material>();
    mMaterials[mat.name.c_str()] = newMat;
    glm::vec4 metFactors;
    metFactors.x = mat.pbrData.metallicFactor;
    metFactors.y = mat.pbrData.roughnessFactor;
    newMat->mData =
        mMaterialBuffer.allocate<interop::MaterialData>(interop::MaterialData{
            .colorFactors = convertVector(mat.pbrData.baseColorFactor),
            .metalRoughnessFactors = metFactors,
        });
    newMat->mPass = mat.alphaMode == fastgltf::AlphaMode::Blend
                        ? Material::Pass::Translucent
                        : Material::Pass::Opaque;

    if (mat.pbrData.baseColorTexture.has_value()) {
      newMat->mTexture = mDescriptorAllocator.allocate<ImageSamplerDescriptor>(
          engine.getTextureDescriptorLayout());
      size_t img =
          asset.textures[mat.pbrData.baseColorTexture.value().textureIndex]
              .imageIndex.value();
      size_t sampler =
          asset.textures[mat.pbrData.baseColorTexture.value().textureIndex]
              .samplerIndex.value();
      newMat->mTexture.write(handle.mDevice,
                             {
                                 .mImage = images[img]->getView(),
                                 .mSampler = mSamplers[sampler],
                             });
    }
  }

  for (auto& mesh : asset.meshes) {
    fmt::println("Mesh {}", mesh.name);
    mMeshes[mesh.name.c_str()] = Mesh::load(asset, mesh);
  }

  // Use three passes: First to convert nodes to our format, then to build the
  // hierarchy. Finally determine root nodes.
  std::vector<std::shared_ptr<Node>> nodes;
  for (auto node : asset.nodes) {
    fmt::println("Node {} -> {}", node.name,
                 node.meshIndex.has_value()
                     ? asset.meshes[node.meshIndex.value()].name
                     : "None");
    auto newNode = std::make_shared<Node>();
    nodes.push_back(newNode);

    if (node.meshIndex.has_value()) {
      newNode->mMesh =
          mMeshes[asset.meshes[node.meshIndex.value()].name.c_str()];
    }

    std::visit(
        fastgltf::visitor{
            [&](fastgltf::math::fmat4x4 matrix) {
              memcpy(&newNode->mLocalTransform, matrix.data(), sizeof(matrix));
            },
            [&](fastgltf::TRS transform) {
              glm::vec3 tl(transform.translation[0], transform.translation[1],
                           transform.translation[2]);
              glm::quat rot(transform.rotation[3], transform.rotation[0],
                            transform.rotation[1], transform.rotation[2]);
              glm::vec3 sc(transform.scale[0], transform.scale[1],
                           transform.scale[2]);

              glm::mat4 tm = glm::translate(glm::mat4(1.f), tl);
              glm::mat4 rm = glm::mat4_cast(rot);
              glm::mat4 sm = glm::scale(glm::mat4(1.f), sc);

              newNode->mLocalTransform.mTransform = tm * rm * sm;
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
}

void GltfMesh::Node::instantiate(ecs::Registry& ecs,
                                 const ecs::MatrixTransform& transform) {
  auto entity = ecs.createEntity();
  auto localModelMat = transform.mTransform * mLocalTransform.mTransform;

  ecs.addComponent<ecs::MatrixTransform>(entity, {localModelMat});
  // TODO: Remove debug hide
  if (mMesh != nullptr && !mName.starts_with("LightShaft")) {
    ecs.addComponent<ecs::Renderable>(entity, {
                                                  .mMesh = mMesh,
                                                  // TODO: Materials
                                              });
    fmt::println("Entity created: {}", mName);
  } else {
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
    root.second->instantiate(ecs, {transform.modelMatrix()});
  }
}

std::unique_ptr<GltfMesh> MeshLoader::loadGltf(Vfs::SubdirPath path) {
  auto asset = loadAsset(path);

  return std::make_unique<GltfMesh>(asset);
}

} // namespace selwonk::vulkan

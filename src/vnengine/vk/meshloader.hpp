#pragma once

#include <fastgltf/core.hpp>
#include <functional>
#include <string>
#include <unordered_map>

#include "../ecs/registry.hpp"

#include "fastgltf/types.hpp"
#include "mesh.hpp"
#include "vncore/handelist.hpp"

namespace selwonk::vulkan {
class GltfMesh {
public:
  GltfMesh(const fastgltf::Asset& asset);
  ~GltfMesh();

  template <typename T>
  using StringMap = std::unordered_map<std::string, std::shared_ptr<T>>;
  void instantiate(ecs::Registry& ecs, const ecs::Transform& transform);

  struct Node {
    Node* mParent;
    std::vector<std::shared_ptr<Node>> mChildren;
    core::HandleList<Mesh>::Handle mMesh;
    ecs::Transform mLocalTransform;
    std::string mName;

    void instantiate(ecs::Registry& ecs, const ecs::Transform& transform);

    void walk(std::function<void(Node&)> visitor) {
      visitor(*this);
      for (auto& child : mChildren) {
        child->walk(visitor);
      }
    }
  };
  StringMap<Node> mRootNodes;

  // TODO: Proper resource management
  // TODO: Probably not needed with nodes now that handles are ref counted
  std::unordered_map<std::string, core::HandleList<Mesh>::Handle> mMeshes;

private:
  static fastgltf::Asset loadAsset(core::Vfs::SubdirPath path);

  static vk::Filter convertFilter(fastgltf::Optional<fastgltf::Filter> filter);
  static vk::SamplerMipmapMode
  convertMipmapMode(fastgltf::Optional<fastgltf::Filter> mode);

  static glm::vec4 convertVector(const fastgltf::math::nvec4& vec);
};

class MeshLoader {
public:
  class LoadException : public std::exception {
  public:
    LoadException(fastgltf::Error error) : mError(error) {}
    fastgltf::Error mError;

    const char* what() const noexcept override {
      return fastgltf::getErrorMessage(mError).data();
    }
  };

  static std::unique_ptr<GltfMesh> loadGltf(core::Vfs::SubdirPath path);

private:
  static fastgltf::Asset loadAsset(core::Vfs::SubdirPath path);
};
} // namespace selwonk::vulkan

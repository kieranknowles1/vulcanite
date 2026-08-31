#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>

#include <vncore/vfs.hpp>

#include <vnecs/registry.hpp>

namespace selwonk::vulkan {
class GltfMesh {
public:
  GltfMesh(std::shared_ptr<fastgltf::Asset> asset);
  ~GltfMesh();

  template <typename T>
  using StringMap = std::unordered_map<std::string, std::shared_ptr<T>>;
  void instantiate(ecs::Registry& ecs, const ecs::Transform& transform);

  struct Node {
    Node* mParent;
    std::vector<std::shared_ptr<Node>> mChildren;
    assets::MeshData::Handle mMesh;
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

private:
  static glm::vec4 convertVector(const fastgltf::math::nvec4& vec);
};

// TODO: This is more of a SceneLoader
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

  static std::unique_ptr<GltfMesh> loadGltf(core::Vfs::FilePtr file);

private:
  static std::shared_ptr<fastgltf::Asset> loadAsset(core::Vfs::FilePtr file);
};
} // namespace selwonk::vulkan

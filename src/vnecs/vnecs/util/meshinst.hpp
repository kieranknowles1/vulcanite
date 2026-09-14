#pragma once
#include <vnecs/registry.hpp>
#include <vnassets/meshloader.hpp>

namespace selwonk::ecs::util {
class MeshInst {
public:
  MeshInst() = delete;

  static void instantiate(Registry& ecs, const assets::GltfMesh& mesh, const Transform& origin);

private:
  static void instantiateNode(Registry& ecs, const assets::GltfMesh::Node& node, const Transform& origin);
};
}
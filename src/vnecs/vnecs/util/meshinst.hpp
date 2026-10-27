#pragma once
#include <vnecs/registry.hpp>
#include <vnassets/meshloader.hpp>

namespace selwonk::ecs::util {
class MeshInst {
public:
  MeshInst() = delete;

  static EntityRef instantiate(Registry& ecs, const assets::GltfMesh& mesh, const Transform& origin);
  static void removeScene(Registry& ecs, EntityRef originEntity);

private:
  static void instantiateNode(Registry& ecs, const assets::GltfMesh::Node& node, const Transform& origin, EntityRef* originEntity);
};
}
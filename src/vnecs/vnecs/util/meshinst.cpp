#include "meshinst.hpp"

namespace selwonk::ecs::util {

void MeshInst::instantiate(Registry& ecs, const assets::GltfMesh& mesh, const Transform& origin)
{
  for (auto& root : mesh.mRootNodes) {
    instantiateNode(ecs, *root.second, origin);
  }
}

void MeshInst::instantiateNode(Registry& ecs, const assets::GltfMesh::Node& node, const Transform& origin)
{
  auto entity = ecs.createEntity();
  // TODO: TRS struct
  Transform localTrans{ node.mTranslation, node.mRotation, node.mScale };
  auto localModelMat = origin.apply(localTrans);

  ecs.addComponent<ecs::Transform>(entity, { localModelMat });
  if (node.mMesh.valid()) {
    ecs.addComponent<ecs::Renderable>(entity, { node.mMesh });
  }

  if (!node.mName.empty()) {
    ecs.addComponent<ecs::Named>(entity, { node.mName });
  }

  for (auto& child : node.mChildren) {
    instantiateNode(ecs, *child, localModelMat);
  }
}

}

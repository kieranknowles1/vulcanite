#include "meshinst.hpp"
#include "vnecs/link.hpp"

namespace selwonk::ecs::util {

EntityRef MeshInst::instantiate(Registry& ecs, const assets::GltfMesh& mesh, const Transform& origin)
{
  EntityRef originEntity;
  for (auto& root : mesh.mRootNodes) {
    instantiateNode(ecs, *root.second, origin, &originEntity);
  }
  return originEntity;
}

void MeshInst::instantiateNode(Registry& ecs, const assets::GltfMesh::Node& node, const Transform& origin, EntityRef* originEntity)
{
  auto entity = ecs.createEntity();
  if (!originEntity->valid()) {
    *originEntity = entity;
  }
  Link link;
  link.set(Link::Type::SceneOrigin, *originEntity);
  ecs.addComponent(entity, link);
  
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
    instantiateNode(ecs, *child, localModelMat, originEntity);
  }
}

void MeshInst::removeScene(Registry& ecs, EntityRef originEntity) {
  // TODO: Delayed delete
  ecs.forEach<const Link&>(
    [&](EntityRef entity, auto link) {
      if (link.get(Link::Type::SceneOrigin) == originEntity) {
        ecs.deleteEntity(entity);
      }
    }
  );
}

}

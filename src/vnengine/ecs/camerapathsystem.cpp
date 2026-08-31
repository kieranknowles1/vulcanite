#include "camerapathsystem.hpp"

#include <chrono>
#include <fmt/base.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "../vk/vulkanengine.hpp"
#include <vnecs/entity.hpp>
#include <vnecs/link.hpp>
#include <vnecs/registry.hpp>
#include <vnecs/transform.hpp>
#include "vncore/times.hpp"
#include "vncore/vfs.hpp"

namespace selwonk::ecs {

CameraPathSystem::CameraPathSystem(EntityRef camera, core::Vfs::FilePtr file)
    : mCamera(camera) {
  std::vector<char> buffer;
  file->readfull(buffer);

  std::vector<Node> nodes = nlohmann::json::parse(buffer);

  // TODO: Create nodes in engine.cpp, move paths to scenes
  auto& ecs = vulkan::VulkanEngine::get().mEcs;

  ecs::EntityRef previous;
  for (auto& node : nodes) {
    auto ent = ecs.createEntity();
    ecs.addComponent(ent, Transform{
                              .mTranslation = node.mPosition,
                              .mRotation = node.mRotation,
                              .mScale = glm::vec3(1.0),
                          });
    if (previous.valid()) {
      ecs.addComponent(previous, Link{.mNext = ent});
    } else {
      // First node
      mCurrentNode = ent;
      mStartingNode = ent;
    }
    previous = ent;
  }
}

CameraPathSystem::~CameraPathSystem() {
  // TODO: Separate system to record paths
  // nlohmann::json json = mNodes;
  // spdlog::("{}", json.dump(2));
}

void CameraPathSystem::update(ecs::Registry& ecs, core::Duration dt) {
  // TODO: DebugDrawSystem for things like this
  auto currDbg = mStartingNode;
  while (ecs.hasComponent<Link>(currDbg)) {
    auto nextDbg = ecs.getComponent<Link>(currDbg).mNext;

    auto& pos1 = ecs.getComponent<Transform>(currDbg);
    auto& pos2 = ecs.getComponent<Transform>(nextDbg);

    vulkan::Debug::get().drawLine(vulkan::Debug::DebugLine{
        .start = pos1.mTranslation,
        .end = pos2.mTranslation,
        .color = vulkan::Debug::Green,
    });
    currDbg = nextDbg;
  }

  // TODO: This should be its own system
  // if (mKeyboard.getDigital(core::Keyboard::DigitalControl::AddCameraNode)) {
  //   auto& transform = ecs.getComponent<Transform>(mCamera);
  //   std::chrono::duration<double> seconds(1.0f);
  //   mNodes.emplace_back(Node{
  //       .mPosition = transform.mTranslation,
  //       .mRotation = transform.mRotation,
  //       .mDuration =
  //           std::chrono::duration_cast<std::chrono::nanoseconds>(seconds),
  //   });
  // }

  // We're done
  if (!mCurrentNode.valid() || !ecs.hasComponent<Link>(mCurrentNode))
    return;

  core::Duration perNodeTime = std::chrono::seconds(1);
  mCurrentNodeTime += dt;
  if (mCurrentNodeTime > perNodeTime) {
    mCurrentNodeTime = core::Duration::zero();
    if (ecs.hasComponent<Link>(mCurrentNode))
      mCurrentNode = ecs.getComponent<Link>(mCurrentNode).mNext;
    else
      mCurrentNode = EntityRef();

    // We're done
    if (!mCurrentNode.valid() || !ecs.hasComponent<Link>(mCurrentNode))
      return;
  }

  float fraction = (float)mCurrentNodeTime.count() / (float)perNodeTime.count();
  assert(fraction >= 0.0f && fraction <= 1.0f);

  auto curr = ecs.getComponent<Transform>(mCurrentNode);
  auto next =
      ecs.getComponent<Transform>(ecs.getComponent<Link>(mCurrentNode).mNext);

  auto newPos = glm::mix(curr.mTranslation, next.mTranslation, fraction);
  auto newRot = glm::slerp(curr.mRotation, next.mRotation, fraction);
  ecs.queueCommand(Transform::SetTransform{.mTarget = mCamera,
                                           .mNewData = {
                                               .mTranslation = newPos,
                                               .mRotation = newRot,
                                               .mScale = glm::vec3(1.0),
                                           }});
}
} // namespace selwonk::ecs

#include "camerapathsystem.hpp"

#include <chrono>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "../vk/vulkanengine.hpp"
#include "registry.hpp"
#include "transform.hpp"
#include "vncore/vfs.hpp"

namespace selwonk::ecs {

CameraPathSystem::CameraPathSystem(EntityRef camera,
                                   const core::Keyboard& keyboard,
                                   core::Vfs::SubdirPath& path)
    : mCamera(camera), mKeyboard(keyboard) {
  auto& vfs = vulkan::VulkanEngine::get().getVfs();
  std::vector<std::byte> buffer;
  vfs.readfull(core::Vfs::Paths / path, buffer);

  mNodes = nlohmann::json::parse(buffer);
}

CameraPathSystem::~CameraPathSystem() {
  // TODO: Separate system to record paths
  nlohmann::json json = mNodes;
  fmt::println("{}", json.dump(2));
}

void CameraPathSystem::update(ecs::Registry& ecs, core::Duration dt) {
  // TODO: This should be its own system
  if (mKeyboard.getDigital(core::Keyboard::DigitalControl::AddCameraNode)) {
    auto& transform = ecs.getComponent<Transform>(mCamera);
    std::chrono::duration<double> seconds(1.0f);
    mNodes.emplace_back(Node{
        .mPosition = transform.mTranslation,
        .mRotation = transform.mRotation,
        .mDuration =
            std::chrono::duration_cast<std::chrono::nanoseconds>(seconds),
    });
  }

  // TODO: Treat as error
  if (mNodes.empty())
    return;

  mCurrentNodeTime += dt;
  while (mCurrentNodeTime >= mNodes[mNodeIndex].mDuration) {
    mCurrentNodeTime -= mNodes[mNodeIndex].mDuration;
    mNodeIndex++;
  }

  if (mNodeIndex >= mNodes.size() - 1)
    return;

  auto curr = mNodes[mNodeIndex];
  auto next = mNodes[mNodeIndex + 1];

  float fraction = (float)mCurrentNodeTime.count() /
                   (float)mNodes[mNodeIndex].mDuration.count();

  auto newPos = glm::mix(curr.mPosition, next.mPosition, fraction);
  // FIXME: This may cause a camera spin, use whichever direction is shortest
  // rotation
  auto newRot = glm::mix(curr.mRotation, next.mRotation, fraction);
  ecs.queueCommand(Transform::SetTransform{.mTarget = mCamera,
                                           .mNewData = {
                                               .mTranslation = newPos,
                                               .mRotation = newRot,
                                               .mScale = glm::vec3(1.0),
                                           }});
}
} // namespace selwonk::ecs

---
category: "[[Bugs]]"
bug-id: 6
bug-fix-commit: 5a7c984a2660def2b2c00612f8ab6ee22e3340d7
bug-subtype: CTD
---
Engine would occasionally crash with `VK_ERROR_DEVICE_LOST` during shutdown. Caused indirectly by destroying the camera's draw target while a command buffer is using it, and resolved by inserting an additional `vkDeviceWaitIdle` before destroying the ECS.
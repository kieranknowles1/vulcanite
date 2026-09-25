---
category: "[[Bugs]]"
bug-id: 7
bug-fix-commit: f0808484fa3d0553a7d2abbf2c51f43884e82590
bug-subtype: Undefined Behaviour
---
The target image descriptor set for the background was updated every frame, which caused it to be updated after bind due to an in-flight command buffer using it, which is undefined behaviour from Vulkan.
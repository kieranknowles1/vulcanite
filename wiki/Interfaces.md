---
category: "[[VNAssets]]"
---
Vulcanite uses several interfaces to expose generic versions of platform-specific functionality. These are defined in [[VNAssets]] and implemented in platform libraries.
# `IController`
Provide user input from either a keyboard, joystick, or other sources.

# `INativeHandleProvider`
Singleton to manage resource handle reference counts and loading.

# `IRenderPipeline`
Common functionality for a renderer and factory for the [[RenderSystem]].
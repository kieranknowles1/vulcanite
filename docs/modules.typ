#import "@preview/diagraph:0.3.7": render

= Modules

#let module(name, namespace) = [
  == #name
  Part of the #raw("selwonk::" + namespace) namespace
]
// TODO: Document these
#module([VNCore], "core")

Core functionality available to all modules. Platform independent.

#module([VNAssets], "assets")

Asset loading and interface declarations. Platform specific behaviour is handled
in // @VNVulkan

#module([VNSdl], "sdl")

SDL implementation of windowing and keyboard input.

#module([VNVulkan], "vulkan")

Vulkan implementation of rendering.

// #module([VNEcs], "ecs")

// TODO: Move to `engine` namespace
#module([VNEngine], "vulkan")

Final engine code, bringing together all subsystems.

#figure(
  render(read("modules.dot")),
  caption: [Engine Modules],
)

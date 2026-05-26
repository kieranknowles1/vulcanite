#import "@preview/diagraph:0.3.7": render

= Modules

#let module(namespace) = [
  Part of the #raw("selwonk::" + namespace) namespace
]
// TODO: Document these
== VNCore <vncore>
#module("core")

Core functionality available to all modules. Platform independent.

== VNAssets
#module("assets")

Asset loading and interface declarations. Platform specific behaviour is handled
in // @VNVulkan

== VNSdl
#module("sdl")

SDL implementation of windowing and keyboard input.

== VNVulkan
#module("vulkan")

Vulkan implementation of rendering.

// #module([VNEcs], "ecs")

== VNEngine <vnengine>
// TODO: Move to `engine` namespace
#module("vulkan")

Final engine code, bringing together all subsystems.

#figure(
  render(read("modules.dot")),
  caption: [Engine Modules],
)

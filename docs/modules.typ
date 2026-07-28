#import "@preview/diagraph:0.3.7": render

= Modules

The vulcanite engine is split into distinct modules. The dependencies between them
are visualised in @module_tree

#let module(namespace) = [
  Part of the #raw("selwonk::" + namespace) namespace
]
// TODO: Document these
== VNCore <vncore>
#module("core")

Core functionality available to all modules. Platform independent.

== VNAssets <vnassets>
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
) <module_tree>

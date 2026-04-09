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

// #module([VNVulkan], "vulkan")

// #module([VNEcs], "ecs")

// TODO: Move to `engine` namespace
#module([VNEngine], "vulkan")

Final engine code, bringing together all subsystems.

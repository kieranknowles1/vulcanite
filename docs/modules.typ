= Modules

#let module(name, namespace) = [
  == #name
  Part of the #raw("selwonk::" + namespace) namespace
]
// TODO: Document these
#module([VNCore], "core")

#module([VNAssets], "assets")

// #module([VNVulkan], "vulkan")

// TODO: Move to `engine` namespace
#module([VNEngine], "vulkan")

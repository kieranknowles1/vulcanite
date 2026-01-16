# Shaders

The default vertex/fragment shaders take the following descriptor bindings:

| Binding | Index | Type                       | Usage                |
| ------- | ----- | -------------------------- | -------------------- |
| 0       | 0     | SceneData                  | Scene level uniforms |
| 0       | 1     | SamplerState[]             | Bindless samplers    |
| 0       | 2     | Texture2D[]                | Bindless textures    |
| 0       | 3     | StructuredBuffer<Vertex>[] | Bindless vertices    |
| 0       | 4     | StructuredBuffer<uint>[]   | Bindless indices     |

This must match the layouts/sets returned by
`VulkanEngine::getStaticDescriptors` and `VulkanEngine::getDescriptorLayouts`,
as well as the bindings declared in shaders.

= Shaders

All shaders are provided with descriptor sets listed in @bindings.
Preferably, these would be variants of the default vertex shader with
different preprocessor options listed in @preprocessor.

#set raw(lang: "hlsl")
#figure(
  table(
    columns: (auto, auto, auto, auto),
    table.header([Binding], [Index], [Type], [Usage]),

    [0], [0], `SceneData`, [Scene level uniforms],

    [0], [1], `SamplerState[]`, [Bindless samplers],

    [0], [2], `Texture2D[]`, [Bindless textures],

    [0], [3], `StructuredBuffer<Vertex>[]`, [Bindless vertex buffers],

    [0], [4], `StructuredBuffer<uint>[]`, [Bindless index buffers],

    [0], [5], `StructuredBuffer<MaterialData>`, [Bindless materials],
  ),
  caption: [Shader bindings, indexes, and types],
) <bindings>

#figure(
  table(
    columns: (auto, auto),
    table.header([Option], [Usage]),

    `NOINDEX`, [Disable using index buffers, instead using vertex IDs directly],

    `NOMAT`, [Disable using material buffers, instead using a common default],
  ),
  caption: [Preprocessor options and their effects],
) <preprocessor>

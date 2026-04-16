= Benchmarks

Benchmarks are performed on the `structure.glb` scene with vsync set to
`LowLatency` and validation layers disabled. These are not at all scientific,
but provide a rough overview of performance over time.

#let bench(ver, gpu, cpu, fps) = {
  (ver, [#str(gpu)ms], [#str(cpu)ms], [#str(fps)FPS])
}

#figure(
  table(
    columns: (auto, auto, auto, auto),
    table.header([Version], [GPU Wait], [CPU Render], [Framerate]),

    ..bench([V1], 0.27, 0.13, 2300),
    ..bench([@bindless_vtx], 0.21, 0.11, 2800),
    ..bench([@pack_uvs], 0.19, 0.11, 2900),
    ..bench([Bindless Indexes], 0.20, 0.11, 2800),
    ..bench([Bindless Samplers/Cache Address], 0.20, 0.11, 2900),
    ..bench([Bindless Textures], 0.20, 0.13, 2800),
    ..bench([Frustum Culling], 0.13, 0.14, 3300),
    ..bench([@true_bindless], 0.13, 0.14, 3400),
    ..bench([Indirect Draw], 0.14, 0.13, 3300),
  ),
  caption: [Benchmarks, and the changes made for them],
)


== Changes Made

// TODO: Link to this section from the table
=== Bindless Vertices <bindless_vtx>
Replace traditional vertex buffers with buffer references, saving significantly
on CPU time.

=== Packed UVs <pack_uvs>

Pack UV into previously unused padding bytes, saving 8 bytes per vertex, saving
a smaller amount on GPU time.

Also accidentally turns the GPU into a speaker with an audible pitch
proportional to the framerate. I can only guess it's equal to FPS and in the
same camp as "I accidentally made an antenna from my cable"

=== True Bindless Vertices <true_bindless>

Use buffers for vertices rather than RawBufferLoads

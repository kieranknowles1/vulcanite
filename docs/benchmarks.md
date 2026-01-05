# Benchmarks

Benchmarks are performed on the `structure.glb` scene with vsync set to
`LowLatency` and validation layers disabled.

| Version           | GPU Wait | CPU Render | Framerate |
| ----------------- | -------- | ---------- | --------- |
| V1                | 0.27ms   | 0.13ms     | 2300FPS   |
| Bindless Vertices | 0.21ms   | 0.11ms     | 2800FPS   |
| Packed UVs        | 0.19ms   | 0.11ms     | 2900FPS   |
| Bindless Indexes  | 0.20ms   | 0.11ms     | 2800FPS   |

## Changes Made

### Bindless Vertices

Replace traditional vertex buffers with buffer references, saving significantly
on CPU time.

### Packed UVs

Pack UV into previously unused padding bytes, saving 8 bytes per vertex, saving
a smaller amount on GPU time.

Also accidentally turns the GPU into a speaker with an audible pitch
proportional to the framerate. I can only guess it's equal to FPS and in the
same camp as "I accidentally made an antenna from my cable"

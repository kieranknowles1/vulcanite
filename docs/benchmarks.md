# Benchmarks

Benchmarks are performed on the `structure.glb` scene with vsync set to
`LowLatency` and validation layers disabled.

| Version           | GPU Wait | CPU Render | Framerate |
| ----------------- | -------- | ---------- | --------- |
| V1                | 0.27ms   | 0.13ms     | 2300FPS   |
| Bindless Vertices | 0.21ms   | 0.11ms     | 2800FPS   |

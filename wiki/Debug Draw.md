A debug draw interface is provided by the `selwonk::assets::Debug` singleton. This allows for immediate-mode drawing of lines, wireframe spheres, wireframe boxes, and solid meshes. All debug draw calls must be repeated every frame.

Rendering is implemented in a backend-specific backend as part of its [[RenderSystem]]. Do not expect the same performance as regular rendering. As the name implies, this is for testing and debugging only.

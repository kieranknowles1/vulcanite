A debug draw interface is provided by the `selwonk::assets::Debug` singleton. This allows for immediate-mode drawing of lines, spheres, boxes, and wireframe meshes. All debug draw calls must be repeated every frame.

Rendering is implemented in a backend-specific backend as part of its [[RenderSystem]].
---
category: "[[Bugs]]"
bug-id: 3
bug-fix-commit: 8dd7acb1db871b12e00414f5438483aaf10674f5
---
Debug meshes would tank performance to single-digit framerates if many were rendered, much more than would be expected from the amount of geometry. Caused by calling `cmd.drawIndirect` after each mesh is written to the draw buffer to redraw all previous meshes. This caused $O(n^2)$ overdraw.
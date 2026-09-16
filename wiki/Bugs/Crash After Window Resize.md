---
category: "[[Bugs]]"
bug-id: 2
bug-fix-commit: d19e2768e1e27379be023fc53f30c59b5c1b50fd
---
Engine would crash after a window resize due to the swap chain being recreated after rendering, which deleted the target images of the current frames' command buffer.
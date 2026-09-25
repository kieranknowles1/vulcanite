---
category: "[[Bugs]]"
bug-id: 4
bug-fix-commit: c397d3fe7a07842581d2a403e7b3116afc0ae236
bug-subtype: Error
---
A ring buffer that was only partially filled would have its sum divided by capacity, rather than the number of known samples.
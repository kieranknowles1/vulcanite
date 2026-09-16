---
category: "[[Bugs]]"
bug-id: 1
bug-fix-commit: 2908158140673e261cbf0479ae33f8f8ea47817e
---
VFS files were opened without `std::ios::binary`. This caused new line characters to be transformed unexpectedly when reading binary files.
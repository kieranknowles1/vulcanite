---
category: "[[Bugs]]"
bug-id: 5
bug-fix-commit: 3f139691c26af0d448f73f3e7ccef464805e866e
bug-subtype: Error
---
The behaviour of Cvar::Enum.dirty() was inverted as it checked for a pending value equal to the current, rather than not equal to.
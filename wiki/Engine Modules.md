---
tags: category
---
![[dependencies.svg]]
> Complete graph of Vulcanite's dependency tree. You can't do that for a Node project.

The Vulcanite engine is split into distinct modules in an attempt to reduce intercoupling, especially when [[Cross Compiling]].
```base
properties:
  file.name:
    displayName: Name
views:
  - type: table
    name: Table
    filters:
      and:
        - category == link("Engine Modules")
    sort: []

```

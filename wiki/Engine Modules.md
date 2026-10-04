---
tags: category
---
![[dependencies.svg]]
> Complete graph of Vulcanite's dependency tree. You can't do that for a Node project.

The Vulcanite engine is split into distinct modules in an attempt to reduce intercoupling, especially when [[Cross Compiling]].

```base
formulas:
  Namespace: html("<code>selwonk::" + escapeHTML(note["module-namespace"]) + "</code>")
properties:
  file.name:
    displayName: Name
views:
  - type: table
    name: Table
    filters:
      and:
        - category == link(this)
    order:
      - file.name
      - formula.Namespace
    sort: []

```
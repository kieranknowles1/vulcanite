---
tags: category
---
No software project is complete without a healthy dose of bugs, and this is no exception.

Bugs in Vulcanite are documented with a short description of the issue, the commit hash where it was resolved, and ideally a new test case to prevent regressions.

```base
properties:
  note.bug-id:
    displayName: ID
  note.bug-fix-commit:
    displayName: Fix Commit
  file.name:
    displayName: Name
views:
  - type: table
    name: Table
    filters:
      and:
        - category == link("Bugs")
        - '!file.path.startsWith("Templates")'
    order:
      - file.name
      - bug-id
      - bug-fix-commit
    sort:
      - property: bug-id
        direction: ASC

```
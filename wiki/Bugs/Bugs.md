---
tags: category
---
No software project is complete without a healthy dose of bugs, and this is no exception.

Bugs in Vulcanite are documented with a short description of the issue, the commit hash where it was resolved, and ideally a new test case to prevent regressions.

```base
formulas:
  fix-commit: link("https://git.selwonk.uk/kieran/vulkanite/commit/" + note["bug-fix-commit"], note["bug-fix-commit"])
properties:
  note.bug-id:
    displayName: ID
  file.name:
    displayName: Name
  formula.fix-commit:
    displayName: Fix Commit
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
      - formula.fix-commit
    sort:
      - property: bug-id
        direction: ASC

```
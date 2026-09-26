Documentation is to be supplied in Obsidian-flavour Markdown with no community plugins installed. Directories shall to be used exclusively for namespaces.

This rule has been through a few iterations, first [MDBook](https://rust-lang.github.io/mdBook/) then [Typst](https://typst.app/), however after adopting it for personal use I believe markdown to be a better fit than typesetting.
# Repository Layout

| Directory     | Usage                            |
| ------------- | -------------------------------- |
| `assets`      | Runtime [[Assets]]               |
| `src`         | [[Vulcanite]] engine souce tree. |
| `third_party` | [[Third Party Dependencies]]     |
| `tools`       | [[Tool Scripts]]                 |
| `wiki`        | This wiki                        |
# Wiki References
Wiki references in code should take the form `(wiki:[[Obsidian Link]])` for searchability and potential automation.
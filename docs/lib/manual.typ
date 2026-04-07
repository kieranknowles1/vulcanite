#import "@preview/abbr:0.3.0"

// TODO: Transform URLs to omit https:// and trailing /

#let _title = title

#let manual(
  title: content,
  // Default programming language for syntax highlighting
  language: str,
  doc,
) = {
  show: abbr.show-rule
  set document(
    title: title,
  )
  show link: underline
  set raw(
    lang: language,
  )
  set table(
    align: left,
    // Lines in tables are ugly
    stroke: (x, y) => (
      bottom: if y == 0 { 0.5pt } else { none },
    ),
  )
  _title(title)
  outline()
  outline(title: [List of Tables], target: figure.where(kind: table))
  // TODO: Exclude list of abbreviations from table of contents
  abbr.list()
  doc
}

#import "@preview/abbr:0.3.0"

// TODO: Transform URLs to omit https:// and trailing /

#let _title = title

// Join a list of elements using the oxford comma
#let oxford-join(items) = {
  let n = items.len()
  if n == 1 {
    items.at(0)
  } else if n == 2 {
    items.at(0) + " and " + items.at(1)
  } else {
    items.slice(0, n - 1).join(sep) + ", and " + items.at(n - 1)
  }
}

#let head-link = it => link(it.target, it.element.body)

#let manual(
  title: content,
  // Default programming language for syntax highlighting
  language: str,
  doc,
) = {
  set document(title: title)

  show: abbr.show-rule

  show link: underline
  set raw(lang: language)
  set heading(numbering: "1.1.")
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
  {
    set heading(outlined: false, numbering: none)
    abbr.list()
  }
  doc
}

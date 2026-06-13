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

#let wip-warning = rect(width: 100%, stroke: red, align(center,[The following describes a work in progress module and is subject to change.]))

// Link to a header by name
// Usage: head-link(<label_id>)
#let head-link = lbl => context {
  let target = query(lbl).at(0)
  link(lbl, target.body)
}

#let manual(
  title: content,
  // Default programming language for syntax highlighting
  language: str,
  doc,
) = {
  set document(title: title)
  set page(
    paper: "a4",
    numbering: "1",
  )

  show: abbr.show-rule

  show link: underline
  set raw(lang: language)
  set table(
    align: left,
    // Lines in tables are ugly
    stroke: (x, y) => (
      bottom: if y == 0 { 0.5pt } else { none },
    ),
  )
  _title(title)
  columns(
    2,
  )[
    #outline()
    #outline(title: [List of Tables], target: figure.where(kind: table))
    #outline(
      title: [List of Figures],
      // Where selectors are awkward as we can't do a NOT, need to manually check
      // for each kind of figure
      target: figure.where(kind: image).or(figure.where(kind: raw)),
    )
    #{
      set heading(outlined: false, numbering: none)
      abbr.list(columns: 1)
    }
  ]
  doc
}

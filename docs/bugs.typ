= Bugs

#let bug-entry((id, title, commit, description)) = {
  let prefix = commit.slice(0, 8)
  let url = "https://github.com/kieranknowles1/vulcanite/commit/" + commit
  (
    [#id],
    title + linebreak() + description,
    link(url, raw(prefix, lang: none)),
  )
}

#let bugs = (
  (
    1,
    [File Read Transforms Newlines],
    "2908158140673e261cbf0479ae33f8f8ea47817e",
    [VFS files were opened without `std::ios::binary`. This caused new line
      characters to be transformed unexpectedly when reading binary files.],
  ),
  (
    2,
    [Crash After Window Resize],
    "d19e2768e1e27379be023fc53f30c59b5c1b50fd",
    [Engine would crash after a window resize due to the swapchain being
      recreated after rendering, deleting the target images of the frames command buffer.],
  ),
)

#figure(
  table(
    columns: (auto, 1fr, auto),
    table.header([ID], [Description], [Fix Commit]),
    ..bugs.fold((), (acc, bug) => acc + bug-entry(bug)),
  ),
  caption: [Known and Resolved Bugs],
) <bugs>

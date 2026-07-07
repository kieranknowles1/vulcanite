= Bugs

#let bug-id = counter("bug")
#let bug-entry((title, commit, description)) = {
  let prefix = commit.slice(0, 8)
  let url = "https://github.com/kieranknowles1/vulcanite/commit/" + commit
  (
    [#bug-id.step()#context bug-id.display()],
    title + linebreak() + description,
    link(url, raw(prefix, lang: none)),
  )
}

#let bugs = (
  (
    [File Read Transforms Newlines],
    "2908158140673e261cbf0479ae33f8f8ea47817e",
    [VFS files were opened without `std::ios::binary`. This caused new line
      characters to be transformed unexpectedly when reading binary files.],
  )
  (
    [Crash After Window Resize],
    "d19e2768e1e27379be023fc53f30c59b5c1b50fd",
    [Engine would crash after a window resize due to the swapchain being
      recreated after rendering, invalidating the command buffer that was about
      to be executed due to its target images being deleted.]
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

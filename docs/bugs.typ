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
  (
    3,
    [Debug Meshes Tank Performance],
    "8dd7acb1db871b12e00414f5438483aaf10674f5",
    [
      Debug meshes would tank performance to single-digit framerates if many were rendered, much more than would be expected from the amount of geometry.
      Caused by calling `cmd.drawIndirect` after each mesh is written to the draw buffer to redraw all previous meshes. This caused O(n^2) overdraw.
    ]
  ),
  (
    4,
    [RingBuffer average includes unset values],
    "c397d3fe7a07842581d2a403e7b3116afc0ae236",
    [
      A ring buffer that was only partially filled would have its sum divided by capacity, rather than the number of known samples.
    ]
  )
)

#figure(
  table(
    columns: (auto, 1fr, auto),
    table.header([ID], [Description], [Fix Commit]),
    ..bugs.fold((), (acc, bug) => acc + bug-entry(bug)),
  ),
  caption: [Known and Resolved Bugs],
) <bugs>

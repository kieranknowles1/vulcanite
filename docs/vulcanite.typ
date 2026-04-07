#import "@preview/abbr:0.3.0"

#show: abbr.show-rule

// TODO: Make this a typst template
#set table(
  align: left,
  stroke: (x, y) => (
    bottom: if y == 0 { 0.5pt } else { none },
  ),
)
#show link: underline

#set document(
  title: [Vulcanite],
)

#title([Vulcanite])

#outline()
#outline(title: [List of Tables], target: figure.where(kind: table))

#abbr.list()
#abbr.make(
  ("ECS", "Entity-Component-System"),
)

= Engine Architecture

The Vulkanite engine is designed, first and foremost, as a learning experience,
don't expect professional quality or performance. It was designed from the start
to use modern industry techniques such as an @ECS, and bindless/GPU-driven
rendering. Some of these behaviours are the unconventional/weird/modern/
"mathematics is discovered not invented" way:

== Reverse Depth Buffers

Depth buffers are reversed from the traditional 1 = far, 0 = camera. This gives
extra precision at a distance rather than wasting it at the near plane, and is
used by at least Godot
#link("https://godotengine.org/article/introducing-reverse-z/").


= Building

#figure(
  table(
    columns: (auto, 1fr),
    table.header([Option], [Description]),

    `VN_LOGALLOCATIONS`,
    [Verbose logging of Vulkan memory allocations. Leaks ignore this option],

    `VN_LOGCOMPONENTSTATS`, [Track number of components of each type],

    `VN_PROFILED`, [Tracy profiler server],
  ),
  caption: [CMake options],
)

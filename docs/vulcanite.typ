#import "lib/manual.typ": head-link, manual

#import "@preview/abbr:0.3.0"

#show: manual.with(
  title: [Vulcanite],
  language: "cpp",
)

#abbr.make(
  ("ECS", "Entity-Component-System"),
  ("CVar", "Console Variable"),
)
// TODO: Automatically create show rules for abbreviations, do in template. Also
// handle plurals
#show "ECS": [@ECS]
#show "CVars": [@CVar:pla]
#show "CVar": [@CVar]

= Engine Architecture

The Vulcanite engine is designed, first and foremost, as a learning experience,
don't expect professional quality or performance. It was designed from the start
to use modern industry techniques such as an ECS, and bindless/GPU-driven
rendering. Some of these behaviours are the unconventional/weird/modern/
"mathematics is discovered not invented" way:

== Reverse Depth Buffers

Depth buffers are reversed from the traditional 1 = far, 0 = camera. This gives
extra precision at a distance rather than wasting it at the near plane, and is
used by at least Godot @godot_reverse_depth.

= Building

Several CMake options, listed in @cmake_options, are provided to enable features
of the engine. These are exposed as C++ macros which are defined across all
modules if the option is enabled.

#figure(
  table(
    columns: (auto, 1fr),
    table.header([Option], [Description]),

    `VN_LOGALLOCATIONS`,
    [Verbose logging of Vulkan memory allocations. Leaks are always logged],

    `VN_LOGCOMPONENTSTATS`, [Track number of components of each type],

    `VN_PROFILED`,
    [Tracy profiler server. See also #head-link(<profiling>) <profiled_opt>],
  ),
  caption: [CMake options],
) <cmake_options>

#include "platforms.typ"
#include "cvar.typ"
#include "shaders.typ"

#include "ecs.typ"
#include "modules.typ"
// TODO: Page on CVars

#include "profiling.typ"
#include "bugs.typ"
#include "todos.typ"

#bibliography("vulcanite.yml")

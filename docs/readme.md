# Template Documentation

## Engine Architecture

The Vulkanite engine is designed, first and foremost, as a learning experience,
don't expect professional quality or performance. It is however based around
modern industry techniques such as ECS (Entity-Component-System).

## CMake Options

- `VN_LOGALLOCATIONS`: Enable verbose logging of Vulkan memory allocations.
  Leaks are always logged, regardless of this setting.
- `VN_LOGCOMPONENTSTATS`: Track and log the number of components of each type,
  for debugging the space efficiency of the ECS.

## The Unconventional

This engine does a few things the unconventional/weird/modern/"mathematics is
discovered not invented way":

### Reverse Depth Buffers

Depth buffers are reversed from the traditional 1 = far, 0 = camera. This gives
extra precision at a distance rather than wasting it at the near plane, and is
used by at least
[Godot](https://godotengine.org/article/introducing-reverse-z/).

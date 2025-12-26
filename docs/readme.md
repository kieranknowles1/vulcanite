# Template Documentation

Whatever this template is about, it is documented here.

## CMake Options

- `VN_LOGALLOCATIONS`: Enable verbose logging of Vulkan memory allocations.
  Leaks are always logged, regardless of this setting.

## The Unconventional

This engine does a few things the unconventional/weird/modern/"mathematics is
discovered not invented way":

### Reverse Depth Buffers

Depth buffers are reversed from the traditional 1 = far, 0 = camera. This gives
extra precision at a distance rather than wasting it at the near plane, and is
used by at least
[Godot](https://godotengine.org/article/introducing-reverse-z/).

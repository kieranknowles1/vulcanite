#import "lib/manual.typ": head-link, wip-warning
#import "@preview/diagraph:0.3.7": render

= Platforms

== Linux <platform_linux>

Presently, only Nix flakes are supported for creating build environments. Run
```sh nix develop``` to enter a dev shell with all dependencies.

== Windows

Windows builds use vcpkg as their package manager. The ```sh CMAKE_TOOLCHAIN_FILE```
environment variable must be set to ```sh $VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake```.

== WebGPU <platform_webgpu>

Emscripten builds create their window in a canvas with ID `vulcanite`. Support
is currently very work in progress. Only Nix host systems are currently supported
through the `wasm` shell. See #head-link(<platform_linux>) for more information.

== Cross-Platform Design
#wip-warning
// TODO: This is all WIP and subject to change
Each rendering backend (currently #head-link(<platform_linux>) and
#head-link(<platform_webgpu>)) operates as a distinct engine module.
#head-link(<vnassets>) declares common functions to decode assets from disk and
interfaces for loader classes. These loaders are expected to return an opaque handle
that may be used by the renderer and ECS.

#figure(
  render(read("modules.dot")),
  caption: [Engine Modules],
)

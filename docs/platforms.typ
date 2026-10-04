#import "lib/manual.typ": head-link, wip-warning
#import "@preview/diagraph:0.3.7": render

= Platforms

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

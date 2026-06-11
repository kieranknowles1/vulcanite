#import "@preview/cheq:0.3.1": checklist
#show: checklist

= TODOs
Features that are yet to be implemented.
#grid(
  columns: 4,
  gutter: 1em,
  [- [ ] Not started], [- [/] Work in Progress], [- [x] Complete], [- [N] No longer planned]
)
// #columns(4, [- [ ] Not started. #colbreak() - [/] WIP.])

- [ ] WebGPU support
  - [/] Promote mesh loading to assets
  - [ ] Promote GLTF loading to assets
  - [ ] `VNEcs` module
  - [ ] Generic engine interface

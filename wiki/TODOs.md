General short to medium term TODOs. See also: [[Wishlist]] for longer term goals.

- [x] Promote mesh loading to assets.
- [x] Promote GLTF loading to assets.
- [x] Move rendering portions of [[VNEngine]] to render interface.
- [x] `VNEcs` module.
- [x] Generic assets layer interface.
- [ ] Replace `#ifdef VN_WASM` instances with enum-like macro for backend.
- [ ] Register CVar validation callbacks before the CLI is parsed. Would need some refactoring as currently systems add callbacks during init. Current design is also unsafe as callbacks may depend on dangling pointers.
- [ ] Generate conventions section on [[Style Guide]] based on conventions sections in other pages. May require dataview.
- [x] Generate glossary page based on note properties.
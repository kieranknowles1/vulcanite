Build procedures are dependant upon the target platform. All builds require git submodules to have been cloned.

# Linux
Only Nix flakes are fully supported, run `nix develop` to enter a dev shell with all dependencies and automatically configure. The `configure` command can be used at any point to rerun CMake.

# Windows
On windows, vcpkg is used to install certain dependencies which must be exposed via the `CMAKE_TOOLCHAIN_FILE` environment variable. Additionally, the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) must be manually installed.

Once dependencies are installed, run the [[Tool Scripts#`configure-wasm`, `configure-win32`|configure-win32]] script to set up a build environment.

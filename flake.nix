{
  description = "A Vulcan-based game engine";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    systems.url = "github:nix-systems/default-linux";

    flake-parts = {
      url = "github:hercules-ci/flake-parts";
      inputs.nixpkgs-lib.follows = "nixpkgs";
    };

    nixcfg = {
      url = "github:kieranknowles1/nixcfg";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.flake-parts.follows = "flake-parts";
      inputs.systems.follows = "systems";

      # Disable inputs not needed outside of nixcfg
      # These can be re-enabled if needed
      inputs.copyparty.follows = "";
      inputs.factorio-blueprints.follows = "";
      inputs.firefox-addons.follows = "";
      inputs.flake-utils.follows = "";
      inputs.home-manager.follows = "";
      inputs.ixx.follows = "";
      inputs.nix-index-database.follows = "";
      inputs.nix-minecraft.follows = "";
      inputs.nixos-cosmic.follows = "";
      inputs.nixos-raspberrypi.follows = "";
      inputs.nixos-raspberrypi-kernellock.follows = "";
      inputs.nixpkgs-stable.follows = "";
      inputs.nuschtosSearch.follows = "";
      inputs.sops-nix.follows = "";
      inputs.src-openmw.follows = "";
      inputs.src-tldr.follows = "";
      inputs.stylix.follows = "";
      inputs.vscode-extensions.follows = "";
    };
  };

  outputs = {
    flake-parts,
    nixcfg,
    ...
  } @ inputs:
    flake-parts.lib.mkFlake {inherit inputs;} {
      systems = import inputs.systems;

      imports = [
        # Can be overridden with https://flake.parts/options/treefmt-nix options
        nixcfg.flakeModules.treefmt
        ./docs
      ];

      perSystem = {
        pkgs,
        inputs',
        ...
      }: {
        devShells.default = inputs'.nixcfg.devShells.cmake.override {
          name = "vulkanite";
          # Vendored libraries are not listed here
          packages = with pkgs; [
            fmt # Formatting library, great for logging
            glm # Vectors, matrices, quaternions, and more
            sdl3 # Windowing and input
            (imgui.override {
              IMGUI_BUILD_VULKAN_BINDING = true;
              IMGUI_BUILD_SDL3_BINDING = true;
            }) # Simple GUI
            simdjson # Dependency of fastgltf
            tinyobjloader # OBJ model loader
            vulkan-headers
            vulkan-loader
            vulkan-utility-libraries
            vulkan-validation-layers
            vulkan-memory-allocator # Malloc for the GPU

            directx-shader-compiler # We use HLSL shaders
            tracy # Frame profiler. Version MUST match .gitmodules
          ];
        };
      };
    };
}

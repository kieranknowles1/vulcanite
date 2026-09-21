#!/usr/bin/env nu

# Script to detect unintentional includes that will break when cross-compiling

# vnengine is not allowed to use Vulkan directly, this must be moved to vnvulkan
rg 'vulkan\.hpp|vk::' src/vnengine

# vnvulkan should use vulkan.hpp patterns over C patterns
rg '[vV]k[A-Z]' src/vnvulkan

# VN_WASM is deprecated in favor of VN_RENDERER
rg VN_WASM

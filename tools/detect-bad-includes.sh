#!/usr/bin/env bash
set -euo pipefail

# Script to detect unintentional includes that will break when cross-compiling

# vnengine is not allowed to use Vulkan directly, this must be moved to vnvulkan
rg 'vulkan\.hpp|vk::' src/vnengine

# VN_WASM is deprecated in favor of VN_RENDERER
rg VN_WASM

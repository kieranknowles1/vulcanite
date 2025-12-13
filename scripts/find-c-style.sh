#!/usr/bin/env bash
set -euo pipefail

SCOPE=( src/**/* )

# Find files using c-style vulkan interfaces

# Should be including "vulkan.hpp"
rg "vulkan.h>" "${SCOPE[@]}"

# Should be using `vk::` versions
rg vk[A-Z] "${SCOPE[@]}"

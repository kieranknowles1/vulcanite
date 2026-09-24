#!/usr/bin/env nu

def query [
  pattern: string,
  scope: string,
] {
  try {
    rg $pattern --glob !third_party/*/**/* $scope
  }
}

# Script to detect unintentional includes that will break when cross-compiling
def main [] {
  # vnengine is not allowed to use Vulkan directly, this must be moved to vnvulkan
  query 'vulkan\.hpp|vk::' src/vnengine

  # vnvulkan should use vulkan.hpp patterns over C patterns
  query '[vV]k[A-Z]' src/vnvulkan

  # VN_WASM is deprecated in favor of VN_RENDERER
  query VN_WASM .
}

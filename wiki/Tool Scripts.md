---
category: "[[Development]]"
---
A collection of shell scripts are provided to perform common tasks, written as Bash, Batch, or NuShell depending on supported platforms.
# Cross Platform
To be written in Nu.
## `detect-bad-includes`
Identifiy lingering usage of deprecated include patterns, such as including vulkan in [[Vulcanite#vnvulkan]].
# Linux
To be written in Bash. Try to make scripts [[#Cross Platform]] if practical.
## `valgrind`
Run a fixed sequence under Valgrind to detect memory leaks and other undefined behaviour.
# Windows
To be written as Batch files. Try to make scripts [[#Cross Platform]] if practical.
## `configure-wasm`, `configure-win32`
Configure a build environment. Linux uses Nix flakes for this.
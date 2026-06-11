#!/usr/bin/env bash
set -euo pipefail

JOBS=8

make -C build -j$JOBS vulcanite

exec valgrind --leak-check=full ./build/vulcanite debug.quit_after 1 core.data_directory build/assets

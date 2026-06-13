#!/usr/bin/env bash
set -euo pipefail

JOBS=8
TOOLS=$(dirname "${BASH_SOURCE}")

make -C build -j$JOBS vulcanite

exec valgrind --leak-check=full --suppressions=$TOOLS/valgrind.supp --gen-suppressions=all \
  ./build/vulcanite debug.quit_after 1 core.data_directory build/assets

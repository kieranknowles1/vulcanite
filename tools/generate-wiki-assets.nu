#!/usr/bin/env nu

def main [] {
  do {
    cd build
    cmake --graphviz=graph.dot .
  }
  # TODO: Could we include internal links on generated SVGs?
  mkdir wiki/Media/Generated
  dot -Tsvg -o wiki/Media/Generated/dependencies.svg build/graph.dot
}

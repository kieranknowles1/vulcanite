#!/usr/bin/env nu

const InjectedStyle = "
@media (prefers-color-scheme: dark) {
  text {
    font-family: Arial, Helvetica, sans-serif;
  }
  svg {
    filter: invert();
    mix-blend-mode: screen;
  }
}
"

def mksvg [
  src: string
  dst: string
] {
  dot -Tsvg -o $dst $src
  open $dst | str replace "</svg>" $"<style>($InjectedStyle)</style></svg>" | save --force $dst
}

def main [] {
  do {
    cd build
    cmake --graphviz=graph.dot .
  }
  # TODO: Could we include internal links on generated SVGs?
  mkdir wiki/Media/Generated
  mksvg build/graph.dot wiki/Media/Generated/dependencies.svg
}

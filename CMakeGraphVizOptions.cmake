set(GRAPHVIZ_IGNORE_TARGETS
  # Implementation detail of libraries that require threading
  "^Threads::Threads$"
  # Implementation detail of SDL3
  "^SDL3::Headers$"

  # Unused vendored library options
  "^fmt-c$"
  "^fmt-header-only"
  "^glm$"
  "^spdlog_header_only$"
)

# TODO: Maybe include these on the individual component pages for reference of
# where it's used and what it uses
set(GRAPHVIZ_GENERATE_PER_TARGET OFF)
set(GRAPHVIZ_GENERATE_DEPENDERS OFF)

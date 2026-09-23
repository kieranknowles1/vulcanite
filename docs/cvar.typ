#import "@preview/mmdr:0.2.2": mermaid
#import "lib/manual.typ": head-link

= Console Variables

== Implementation

CVars are defined in #head-link(<vncore>), and are therefore available to all
executables. The GUI for rendering CVars is implemented in #head-link(<vnengine>)
to avoid introducing a dependency between VNCore and ImGui. See @cvar_diagram for
the class diagram of variables.

// TODO: Fix ugly layout here, may need new mmdr version. Store should be above entry
#figure(
  mermaid(read("cvar.mermaid")),
  caption: [CVar Class Diagram],
) <cvar_diagram>

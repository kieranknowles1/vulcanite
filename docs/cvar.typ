#import "@preview/mmdr:0.2.2": mermaid
#import "lib/manual.typ": head-link

= Console Variables

CVars are variables that can be modified at runtime or on the command line to
control systems. They are defined as static variables and automatically
registered on startup.

Validation callbacks can be registered on a variable to reject nonsensical values,
this takes the form of a lambda that returns `std::nullopt` for a valid value,
or a string explaining the error on an invalid one. WARNING: Validation callbacks are
not currently checked if a variable is set by the console.
// TODO: Validations should be checked if arg passed from console.

Change callbacks are executed immediately after a variable is updated and should
be used to update a system's state to match the new configuration. Where a change
callback is not suitable, apply the `InitOnly` flag to block changes until the
engine is restarted.

A variable can be an `Int`, `Float`, `Bool`, `String`, or an `Enum`. Simple types
are declared with their name, default value, description, and optional flags.

An `Enum`, in addition to a name, default, and description, requires a mapping of
names plus descriptions of their values. An empty string for a description will be treated as null.

A variable may take its default value from a function if using a compile-time constant
is unsuitable. In this case the default should be described using JavaScript
template literal notation (`${interopolated}`).

@cvar_samples provides an example of how to declare each type of CVar.

#figure(
  ```cpp
  // Simple
  Cvar::Int WindowWidth("window.width", 1280, "Window width", Cvar::Flags::Unsigned);
  Cvar::Int WindowHeight("window.height", 720, "Window height", Cvar::Flags::Unsigned);

  // Enum
  Cvar::Enum<vk::PresentModeKHR> VsyncMode(
      "render.vsync", vk::PresentModeKHR::eMailbox,
      "VSync mode, prevents screen tearing",
      {
          {"None", "Present frames immediately, may cause tearing",
           vk::PresentModeKHR::eImmediate},
          {"LowLatency", "", vk::PresentModeKHR::eMailbox},
          ...
      });


  // Dynamic
  static std::string defaultDataDir() { ... }
  Cvar::String DataDirectory("core.data_directory", defaultDataDir,
                             "${exe_directory}/assets",
                             "Path of data directory",
                             Cvar::Flags::InitOnly);
  ```,
  caption: [Simple, enum, and dynamic CVars],
) <cvar_samples>

Variables may be passed on the command line using a simple key-value format.
```sh
./vulcanite var.name value
```

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

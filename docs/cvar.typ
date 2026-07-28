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

A variable can be an `Int`, `Float`, `Bool`, `String`, or an `Enum` and should be
defined as a static variable. Systems are responsible for registering callbacks
during startup.

```cpp
Cvar::Int WindowWidth("window.width", 1280, "Window width", Cvar::Flags::Unsigned);
Cvar::Int WindowHeight("window.height", 720, "Window height", Cvar::Flags::Unsigned);
```

An `Enum`, in addition to a name, default, and description, requires a mapping of
names plus optional descriptions to values.

```cpp
Cvar::Enum<vk::PresentModeKHR> VsyncMode(
    "render.vsync", vk::PresentModeKHR::eMailbox,
    "VSync mode, prevents screen tearing",
    {
        {"None", "Present frames immediately, may cause tearing",
         vk::PresentModeKHR::eImmediate},
        // An empty string for description will be treated as none
        {"LowLatency", "", vk::PresentModeKHR::eMailbox},
    });
```

== Implementation

CVars are defined in #head-link(<vncore>), and are therefore available to all
executables. The GUI for rendering CVars is implemented in #head-link(<vnengine>)
to avoid introducing a dependency between VNCore and ImGui. See @cvar_diagram for
the class diagram of variables.

// TODO: Fix ugly layout here, may need new mmdr version
#figure(
  mermaid(read("cvar.mermaid")),
  caption: [CVar Class Diagram],
) <cvar_diagram>

= Console Variables

CVars are variables that can be modified at runtime or on the command line to
control systems. They are defined as static variables and automatically
registered on startup.

Validation callbacks can be registered on a variable to reject nonsensical values,
this takes the form of a lambda that returns `std::nullopt` for a valid value,
or a string explaining the error on an invalid one.

Change callbacks are executed immediately after a variable is updated and should
be used to update a system's state to match the new configuration. Where a change
callback is not suitable, apply the `InitOnly` flag to block changes until the
engine is restarted.

```cpp
Cvar::Int WindowWidth("window.width", 1280, "Window width");
Cvar::Int WindowHeight("window.height", 720, "Window height");
```

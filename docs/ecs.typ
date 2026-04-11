#import "lib/manual.typ": head-link, oxford-join

= Entities, Components, and Their Systems

== Adding a Component

Define a struct for the new component, the following static fields are expected:

- `const static constexpr ComponentType Type`: Enum value for the component's type. MUST be unique
  per-component.
- `const static constexpr char* Name`: Debug name for the component
- `using Store`: Container to store this component in. Should be `ComponentArray<T>` for
  commonly used components, and `SparseComponentArray<T>` for rarely used ones.

Add an entry for the new component to the `ComponentType` enum.

Add `NewComponent::Store` to `Registry::ComponentArrayTuple`.

== Entities

An entity does nothing on its own; it is simply an ID that can reference
components. The ID of an entity is guaranteed to never change. Deleted IDs may
be reused in future, though this is not yet the case.

== Components

A component may declare `const void onEcsAdd() const` and
`const void onEcsRemove() const` to handle addition and removal to/from the ECS.
This can be used to update ref counts for handles that they own.

=== Flags

Flags represent a single bit of information: set or unset.

Each entity has an `Alive` flag, set for its entire lifetime Deleted entities
have their `Alive` flag set to false, and cannot be revived. The `Enabled`
flag may be toggled at any time, a disabled entity will be ignored by
systems that do not opt-in to updating them.

=== Transform <transform>

A position, rotation, and scale in 3D space.

=== Named <named>

A short name to identify the component.

=== Renderable <renderable>

A mesh and material to be rendered.

=== Camera <camera>

An image that will be drawn to each frame.

=== Link <link>

Link to another entity in a chain

== Systems
// TODO: Only convert first reference in a section into a link
Operations on the ECS should be performed through systems. Each system derives
from the `ecs::System` class and provides an update method which is called every
frame. Systems have a read-only view of the ECS during updates, all modifications
must be performed via commands which are queued until application with an
`ApplyCommandsSystem`, see @apply_commands.

#let system(name, deps) = [
  #show ref: head-link
  === #name
  Operates on entities with #oxford-join(deps.map(d => ref(d))) components.
]

=== ApplyCommandsSystem <apply_commands>
// TODO: Document this, state how the engine is designed to be multi-threaded and
// that systems have read-only access. Mention how to add new command types

=== CameraSystem

// TODO: Properly formatted System function that includes a list of dependant components,
// and whether it is a barrier
// Control a single camera with keyboard and mouse movement.

#system([RenderSystem], (<transform>, <renderable>))

// Render all entities with both [Transform](#Transform) and
// [Renderable](#Renderable) components to all cameras.

=== CameraPathSystem

// TODO: Change to FollowPathSystem

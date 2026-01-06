# Entities, Components, and Their Systems

## Adding a Component

Define a struct for the new component, the following static fields are expected:

- `ComponentType Type`: Enum value for the component's type. MUST be unique
  per-component.
- `char* Name`: Debug name for the component
- `Store`: Container to store this component in. Should be `ComponentArray` for
  commonly used components, and `SparseComponentArray` for rarely used ones.

Add an entry for the new component to the `ComponentType` enum.

Add `NewComponent::Store` to `Registry::ComponentArrayTuple`.

## Entities

An entity does nothing on its own; it is simply an ID that can reference
components. The ID of an entity is guaranteed to never change. Deleted IDs may
be reused in future, though this is not yet the case.

### Flags

Each entity has an `Alive` flag, set for its entire lifetime. And an `Enabled`
flag that may be toggled at any time, a disabled entity will be ignored by
systems that do not opt-in to updating them.

## Components

### Transform

A position, rotation, and scale in 3D space.

### Named

A short name to identify the component.

### Renderable

A mesh and material to be rendered.

### Camera

An image that will be drawn to each frame.

## Systems

### Rendering

Render all entities with both [Transform](#Transform) and
[Renderable](#Renderable) components.

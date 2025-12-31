# Entities, Components, and Their Systems

## Adding a Component

Define a struct for the new component, the following static constexpr fields are
expected:

- `ComponentType Type`: Enum value for the component's type
- `char* Name`: Debug name for the component

Add an entry for the new component to the `ComponentType` enum.

Add `ComponentArray<NewComponent>` to `Registry::ComponentArrayTuple`.

## Entities

An entity does nothing on its own, it's simply an ID that can reference
components.

## Components

### Transform

A position, rotation, and scale in 3D space.

### Renderable

A mesh and material to be rendered.

## Systems

### Rendering

Render all entities with both [Transform](#Transform) and
[Renderable](#Renderable) components.

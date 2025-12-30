# Entities, Components, and Their Systems

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

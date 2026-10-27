---
category: "[[VNEcs]]"
---
A component can optionally include hooks called on specific events. These do not have access to the ECS itself so **must not** mutate ECS state. These should be exposed as public member functions of the component struct.
# onEcsAdd
Fired when the component is initially added.
# onEcsRemove
Fired when the component is removed, or the entity owning it is deleted.
#pragma once

#include <cstdint>
#include <limits>

namespace selwonk::ecs {
using EntityId = uint32_t;
const static constexpr EntityId InvalidEntityId =
    std::numeric_limits<EntityId>::max();
} // namespace selwonk::ecs

#pragma once

#include <cstdint>
#include <vector>

namespace sw::features::domain
{
    struct PositionOffset
    {
        int32_t x{0};
        int32_t y{0};
    };

    struct PositionOccupier
    {
        std::vector<PositionOffset> offsets{{0, 0}};
    };
}

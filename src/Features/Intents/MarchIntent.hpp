#pragma once

#include "Core/Pipeline/Intent.hpp"
#include "Core/Domain/Position.hpp"

#include <cstdint>

namespace sw::features::intents
{
    struct MarchIntent : public core::pipeline::Intent
    {
        uint32_t unitId;

        core::domain::Position posFrom;
        core::domain::Position posTo;

        MarchIntent(uint32_t id, core::domain::Position from, core::domain::Position to)
            : unitId(id), posFrom(from), posTo(to) {}
    };
}

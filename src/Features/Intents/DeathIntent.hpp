#pragma once

#include <cstdint>

#include "Core/Pipeline/Intent.hpp"

namespace sw::features::intents
{
    struct DeathIntent : public core::pipeline::Intent
    {
        uint32_t unitId;

        explicit DeathIntent(uint32_t id)
            : unitId(id) {}
    };
}

#pragma once

#include <cstdint>

#include "Core/Pipeline/Intent.hpp"

namespace sw::features::intents
{
    struct RangedAttackIntent : public core::pipeline::Intent
    {
        uint32_t attackerId;
        uint32_t targetId;

        RangedAttackIntent(uint32_t attacker, uint32_t target)
            : attackerId(attacker), targetId(target) {}
    };
}

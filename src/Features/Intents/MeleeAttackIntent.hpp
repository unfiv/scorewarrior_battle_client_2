#pragma once

#include <cstdint>

#include "Core/Pipeline/Intent.hpp"

namespace sw::features::intents
{
    struct MeleeAttackIntent : public core::pipeline::Intent
    {
        uint32_t attackerId;
        uint32_t targetId;

        MeleeAttackIntent(uint32_t attacker, uint32_t target)
            : attackerId(attacker), targetId(target) {}
    };
}

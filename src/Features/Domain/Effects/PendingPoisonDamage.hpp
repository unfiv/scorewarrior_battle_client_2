#pragma once

#include <cstdint>
#include <unordered_map>

namespace sw::features::domain::effects
{
	using PendingPoisonDamage = std::unordered_map<uint32_t, uint32_t>;
}

#pragma once

#include "Core/World.hpp"
#include "Features/Domain/Effects/EffectData.hpp"
#include "Features/Intents/DamageIntent.hpp"

#include <any>

namespace sw::features::systems::effects
{
	class PoisonEffect
	{
	public:
		static void apply(core::World& world, uint32_t targetId, domain::effects::ActiveEffect& effect)
		{
			constexpr uint32_t kPoisonTicks = 5;

			auto& data = std::any_cast<domain::effects::PoisonEffectData&>(effect.data);
			const uint32_t baseDamage = data.totalDamage / kPoisonTicks;
			const uint32_t remainder = data.totalDamage % kPoisonTicks;

			uint32_t tickDamage = baseDamage;
			if (data.appliedTicks < remainder)
			{
				++tickDamage;
			}

			if (world.getComponent<domain::effects::RendingEffectData>().contains(targetId))
			{
				tickDamage *= 2;
			}

			if (tickDamage > 0)
			{
				world.pushIntent(
						std::make_unique<intents::DamageIntent>(effect.sourceUnitId, targetId, tickDamage, "poison"));
			}

			++data.appliedTicks;
		}
	};
}

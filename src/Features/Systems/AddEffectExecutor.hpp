#pragma once

#include "Core/World.hpp"
#include "Features/Domain/Effects/EffectData.hpp"
#include "Features/Domain/Effects/EffectFactory.hpp"
#include "Features/Domain/Effects/EffectImmunity.hpp"
#include "Features/Domain/Effects/EffectList.hpp"
#include "Features/Intents/AddEffectIntent.hpp"
#include "Features/Systems/Effects/PoisonEffect.hpp"
#include "Features/Systems/Effects/RendingEffect.hpp"

namespace sw::features::systems
{
	class AddEffectExecutor
	{
	public:
		static void execute(core::World& world, intents::AddEffectIntent& intent)
		{
			auto& immunity = world.getComponent<domain::effects::EffectImmunity>();
			if (immunity.contains(intent.targetId))
			{
				return;
			}

			auto& effects = world.getComponent<domain::effects::EffectList>()[intent.targetId].active;

			switch (intent.effectType)
			{
				case intents::EffectType::Poison:
					effects.push_back(
							domain::effects::create(
									domain::effects::PoisonEffectData{intent.damage, 0},
									intent.duration,
									intent.sourceId,
									effects::PoisonEffect::apply));

					// Poison starts ticking immediately on application.
					if (!effects.empty())
					{
						auto& poisonEffect = effects.back();
						poisonEffect.applyFn(world, intent.targetId, poisonEffect);
						if (poisonEffect.remainingTicks > 0)
						{
							--poisonEffect.remainingTicks;
						}
						if (poisonEffect.remainingTicks == 0)
						{
							effects.pop_back();
						}
					}
					break;
				case intents::EffectType::Rending:
					effects.push_back(
							domain::effects::create(
									domain::effects::RendingEffectData{intent.damage},
									intent.duration,
									intent.sourceId,
									effects::RendingEffect::apply));
					world.getComponent<domain::effects::RendingEffectData>()[intent.targetId] = {intent.damage};
					break;
			}
		}
	};
}

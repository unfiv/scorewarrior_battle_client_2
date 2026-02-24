#pragma once

#include "Core/World.hpp"
#include "Features/Domain/Health.hpp"
#include "Features/Events/UnitAttacked.hpp"
#include "Features/Intents/DeathIntent.hpp"
#include "Features/Intents/DamageIntent.hpp"

namespace sw::features::systems::DamageSystem
{
	static void execute(core::World& world, intents::DamageIntent& intent)
	{
		auto& healthMap = world.getComponent<domain::Health>();
		auto target = healthMap.find(intent.targetId);
		if (target == healthMap.end() || target->second.hp == 0)
		{
			return;
		}

		target->second.hp = (target->second.hp > intent.damage) ? (target->second.hp - intent.damage) : 0;
		world.getEvents().event(
				world.getTick(),
				events::UnitAttacked{intent.attackerId, intent.targetId, intent.damage, target->second.hp, intent.type});

		if (target->second.hp == 0)
		{
			world.pushIntent(std::make_unique<intents::DeathIntent>(intent.targetId));
		}
	}
}

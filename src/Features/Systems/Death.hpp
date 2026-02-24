#pragma once

#include "Core/Services/EntityManager.hpp"
#include "Core/World.hpp"
#include "Features/Domain/Effects/EffectList.hpp"
#include "Features/Domain/Health.hpp"
#include "Features/Domain/Melee.hpp"
#include "Features/Domain/PoisonAbility.hpp"
#include "Features/Domain/Ranged.hpp"
#include "Features/Domain/RendingAbility.hpp"
#include "Features/Intents/DeathIntent.hpp"

#include <vector>

namespace sw::features::systems
{
	class Death
	{
	public:
		static std::unique_ptr<intents::DeathIntent> plan(core::World& world, uint32_t id)
		{
			auto& healthMap = world.getComponent<domain::Health>();
			auto health = healthMap.find(id);
			if (health != healthMap.end() && health->second.hp == 0)
			{
				return std::make_unique<intents::DeathIntent>(id);
			}

			return nullptr;
		}

		static void execute(core::World& world, intents::DeathIntent& intent)
		{
			destroy(world, intent.unitId);
		}

	private:
		static void destroy(core::World& world, uint32_t id)
		{
			core::services::EntityManager::destroy(world, id, [&](uint32_t targetId) {});
		}
	};
}

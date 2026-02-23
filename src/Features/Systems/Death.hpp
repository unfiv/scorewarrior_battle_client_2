#pragma once

#include <vector>

#include "Core/World.hpp"
#include "Core/UnitManager.hpp"
#include "Features/Domain/Health.hpp"
#include "Features/Domain/Melee.hpp"
#include "Features/Domain/RendingAbility.hpp"
#include "Features/Domain/Ranged.hpp"
#include "Features/Domain/PoisonAbility.hpp"
#include "Features/Domain/Effects/EffectList.hpp"
#include "Features/Intents/DeathIntent.hpp"

namespace sw::features::systems
{
    class Death
    {
    public:
        static std::shared_ptr<intents::DeathIntent> plan(core::World& world, uint32_t id)
        {
            auto& healthMap = world.getComponent<domain::Health>();
            auto health = healthMap.find(id);
            if (health != healthMap.end() && health->second.hp == 0)
            {
                return std::make_shared<intents::DeathIntent>(id);
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
            core::UnitManager::destroy(world, id, [&](uint32_t targetId)
            {
                world.getComponent<domain::Health>().erase(targetId);
                world.getComponent<domain::Melee>().erase(targetId);
                world.getComponent<domain::RendingAbility>().erase(targetId);
                world.getComponent<domain::Ranged>().erase(targetId);
                world.getComponent<domain::PoisonAbility>().erase(targetId);
                world.getComponent<domain::effects::EffectList>().erase(targetId);
            });
        }
    };
}

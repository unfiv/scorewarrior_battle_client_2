#pragma once

#include <random>
#include <vector>

#include "Core/World.hpp"
#include "Core/Systems/Spatial.hpp"

#include "Features/Domain/Health.hpp"
#include "Features/Domain/Melee.hpp"
#include "Features/Domain/RendingAbility.hpp"
#include "Features/Events/UnitAbilityUsed.hpp"
#include "Features/Intents/DamageIntent.hpp"
#include "Features/Systems/Effects.hpp"
#include "Features/Systems/Effects/RendingEffect.hpp"

namespace sw::features::systems
{
    class MeleeAttack
    {
    public:
        static void processUnit(core::World& world, uint32_t attackerId)
        {
            if (!world.restrictions.isAllowed(attackerId, core::registry::restrictions::ATTACK))
            {
                return;
            }

            auto& healthMap = world.getComponent<domain::Health>();
            auto& meleeMap = world.getComponent<domain::Melee>();

            if (healthMap.find(attackerId) == healthMap.end() ||
                meleeMap.find(attackerId) == meleeMap.end() ||
                healthMap[attackerId].hp == 0)
            {
                return;
            }

            auto attackerPos = world.positions[attackerId];
            std::vector<uint32_t> targets;
            core::systems::Spatial::findTargets(world, attackerId, attackerPos, targets);

            std::vector<uint32_t> aliveTargets;
            for (uint32_t id : targets)
            {
                if (healthMap.count(id) && healthMap[id].hp > 0)
                {
                    aliveTargets.push_back(id);
                }
            }

            if (aliveTargets.empty())
            {
                return;
            }

            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<size_t> targetDistribution(0, aliveTargets.size() - 1);
            uint32_t targetId = aliveTargets[targetDistribution(gen)];

            world.restrictions.modify(attackerId, core::registry::restrictions::MOVE, 1);
            world.restrictions.modify(attackerId, core::registry::restrictions::ATTACK, 1);
            executeAttack(world, attackerId, targetId);
        }

    private:
        static void executeAttack(core::World& world, uint32_t attackerId, uint32_t targetId)
        {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_int_distribution<> dis(1, 1000);

            auto& attackerMelee = world.getComponent<domain::Melee>()[attackerId];
            auto& rendingAbilities = world.getComponent<domain::RendingAbility>();

            uint32_t damage = attackerMelee.strength;
            if (auto ability = rendingAbilities.find(attackerId); ability != rendingAbilities.end())
            {
                if (dis(gen) <= ability->second.chance)
                {
                    damage = ability->second.rending;
                    Effects::addEffect(world, targetId, effects::RendingEffect::create(attackerId, 0));
                    world.getEvents().event(world.getTick(), events::UnitAbilityUsed{attackerId, "rending"});
                }
            }

            world.pushIntent(std::make_shared<intents::DamageIntent>(attackerId, targetId, damage, "melee"));
        }
    };
}

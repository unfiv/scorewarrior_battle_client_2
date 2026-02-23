#pragma once

#include <random>
#include <vector>

#include "Core/World.hpp"
#include "Core/Domain/Position.hpp"

#include "Features/Domain/Health.hpp"
#include "Features/Domain/Melee.hpp"
#include "Features/Domain/MeleeAttackable.hpp"
#include "Features/Domain/PositionOccupier.hpp"
#include "Features/Domain/RendingAbility.hpp"
#include "Features/Events/UnitAbilityUsed.hpp"
#include "Features/Intents/DamageIntent.hpp"
#include "Features/Systems/MarchSystem.hpp"
#include "Features/Systems/Effects.hpp"
#include "Features/Systems/Effects/RendingEffect.hpp"
#include "Features/Intents/MeleeAttackIntent.hpp"

namespace sw::features::systems
{
    class MeleeAttack
    {
    public:
        static std::shared_ptr<intents::MeleeAttackIntent> plan(core::World& world, uint32_t attackerId)
        {
            auto& healthMap = world.getComponent<domain::Health>();
            auto& meleeMap = world.getComponent<domain::Melee>();

            if (healthMap.find(attackerId) == healthMap.end() ||
                meleeMap.find(attackerId) == meleeMap.end() ||
                healthMap[attackerId].hp == 0)
            {
                return nullptr;
            }

            auto& positions = world.getComponent<core::domain::Position>();
            auto& meleeAttackableMap = world.getComponent<domain::MeleeAttackable>();
            auto positionIt = positions.find(attackerId);
            if (positionIt == positions.end())
            {
                return nullptr;
            }

            auto attackerPos = positionIt->second;
            std::vector<uint32_t> targets;
            MarchSystem::findTargets(world, attackerId, attackerPos, targets);

            std::vector<uint32_t> aliveTargets;
            for (uint32_t id : targets)
            {
                if (healthMap.count(id) && healthMap[id].hp > 0 && meleeAttackableMap.count(id))
                {
                    aliveTargets.push_back(id);
                }
            }

            if (aliveTargets.empty())
            {
                return nullptr;
            }

            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<size_t> targetDistribution(0, aliveTargets.size() - 1);
            uint32_t targetId = aliveTargets[targetDistribution(gen)];

            return std::make_shared<intents::MeleeAttackIntent>(attackerId, targetId);
        }

        static void execute(core::World& world, intents::MeleeAttackIntent& intent)
        {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_int_distribution<> dis(1, 1000);

            auto attackerId = intent.attackerId;
            auto targetId = intent.targetId;
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

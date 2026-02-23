#pragma once

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

#include "Core/World.hpp"
#include "Core/Domain/Position.hpp"
#include "Features/Domain/Health.hpp"
#include "Features/Domain/PositionOccupier.hpp"
#include "Features/Domain/Ranged.hpp"
#include "Features/Domain/PoisonAbility.hpp"
#include "Features/Events/UnitAbilityUsed.hpp"
#include "Features/Systems/Damage.hpp"
#include "Features/Systems/Effects.hpp"
#include "Features/Systems/Effects/PoisonEffect.hpp"
#include "Features/Systems/MarchSystem.hpp"
#include "Features/Intents/RangedAttackIntent.hpp"

namespace sw::features::systems
{
    class RangedAttack
    {
    public:
        static std::shared_ptr<intents::RangedAttackIntent> plan(core::World& world, uint32_t attackerId)
        {
            if (!world.restrictions.isAllowed(attackerId, core::registry::restrictions::ATTACK))
            {
                return nullptr;
            }

            auto& healthMap = world.getComponent<domain::Health>();
            auto& rangedMap = world.getComponent<domain::Ranged>();

            if (healthMap.find(attackerId) == healthMap.end() ||
                rangedMap.find(attackerId) == rangedMap.end() ||
                healthMap[attackerId].hp == 0)
            {
                return nullptr;
            }

            auto& positions = world.getComponent<core::domain::Position>();
            auto positionIt = positions.find(attackerId);
            if (positionIt == positions.end())
            {
                return nullptr;
            }

            auto attackerPos = positionIt->second;

            std::vector<uint32_t> neighbors;
            MarchSystem::findTargets(world, attackerId, attackerPos, neighbors);
            for (uint32_t id : neighbors)
            {
                if (healthMap.count(id) && healthMap[id].hp > 0)
                {
                    return nullptr;
                }
            }

            std::vector<uint32_t> targets;
            for (const auto& [targetId, targetPos] : positions)
            {
                if (targetId == attackerId) continue;
                if (!healthMap.count(targetId) || healthMap[targetId].hp == 0) continue;

                uint32_t dist = chebyshevDistance(attackerPos, targetPos);
                if (dist >= 2 && dist <= rangedMap[attackerId].range)
                {
                    targets.push_back(targetId);
                }
            }

            if (targets.empty())
            {
                return nullptr;
            }

            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<size_t> targetDistribution(0, targets.size() - 1);
            uint32_t targetId = targets[targetDistribution(gen)];

            return std::make_shared<intents::RangedAttackIntent>(attackerId, targetId);
        }

        static void execute(core::World& world, intents::RangedAttackIntent& intent)
        {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_int_distribution<> dis(1, 1000);

            auto attackerId = intent.attackerId;
            auto targetId = intent.targetId;
            auto& ranged = world.getComponent<domain::Ranged>()[attackerId];
            auto& poisonAbilities = world.getComponent<domain::PoisonAbility>();

            world.restrictions.modify(attackerId, core::registry::restrictions::MOVE, 1);
            world.restrictions.modify(attackerId, core::registry::restrictions::ATTACK, 1);

            uint32_t damage = ranged.agility;
            if (auto ability = poisonAbilities.find(attackerId); ability != poisonAbilities.end())
            {
                if (dis(gen) <= ability->second.chance)
                {
                    // Add poison effect which will deal total 'poison' damage over 5 ticks.
                    Effects::addEffect(world, targetId, effects::PoisonEffect::create(attackerId, ability->second.poison));
                    world.getEvents().event(world.getTick(), events::UnitAbilityUsed{attackerId, "poison"});

                    // Poison substitutes the main ranged damage: do NOT apply immediate fractional damage here.
                    damage = 0;
                }
            }

            if (damage > 0)
            {
                Damage::apply(world, attackerId, targetId, damage);
            }
        }

        static uint32_t chebyshevDistance(core::domain::Position lhs, core::domain::Position rhs)
        {
            auto dx = std::abs(static_cast<int>(lhs.x) - static_cast<int>(rhs.x));
            auto dy = std::abs(static_cast<int>(lhs.y) - static_cast<int>(rhs.y));
            return static_cast<uint32_t>(std::max(dx, dy));
        }
    };
}

#pragma once

#include "Core/Domain/Position.hpp"
#include "Core/World.hpp"
#include "Features/Domain/Health.hpp"
#include "Features/Domain/PoisonAbility.hpp"
#include "Features/Domain/PositionOccupier.hpp"
#include "Features/Domain/Ranged.hpp"
#include "Features/Domain/RangedAttackable.hpp"
#include "Features/Events/UnitAbilityUsed.hpp"
#include "Features/Intents/AddEffectIntent.hpp"
#include "Features/Intents/DamageIntent.hpp"
#include "Features/Intents/RangedAttackIntent.hpp"
#include "Features/Systems/MarchSystem.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <vector>

namespace sw::features::systems
{
	class RangedAttack
	{
	public:
		static std::unique_ptr<intents::RangedAttackIntent> plan(core::World& world, uint32_t attackerId)
		{
			auto& healthMap = world.getComponent<domain::Health>();
			auto& rangedMap = world.getComponent<domain::Ranged>();

			if (healthMap.find(attackerId) == healthMap.end() || rangedMap.find(attackerId) == rangedMap.end()
				|| healthMap[attackerId].hp == 0)
			{
				return nullptr;
			}

			auto& positions = world.getComponent<core::domain::Position>();
			auto& rangedAttackableMap = world.getComponent<domain::RangedAttackable>();
			auto positionIt = positions.find(attackerId);
			if (positionIt == positions.end())
			{
				return nullptr;
			}

			std::vector<uint32_t> neighbors;
			MarchSystem::findTargets(world, attackerId, neighbors);
			for (uint32_t id : neighbors)
			{
				if (healthMap.count(id) && healthMap[id].hp > 0)
				{
					return nullptr;
				}
			}

			std::vector<uint32_t> targets;
			for (const auto& [targetId, _] : positions)
			{
				if (targetId == attackerId)
				{
					continue;
				}
				if (!healthMap.count(targetId) || healthMap[targetId].hp == 0)
				{
					continue;
				}
				if (!rangedAttackableMap.count(targetId))
				{
					continue;
				}

				const auto& targetRangedAttackable = rangedAttackableMap[targetId];
				int effectiveMinRange = std::max(0, 2 + targetRangedAttackable.minRangeModifier);
				int effectiveMaxRange = std::max(
						0, static_cast<int>(rangedMap[attackerId].range) + targetRangedAttackable.maxRangeModifier);
				if (effectiveMaxRange < effectiveMinRange)
				{
					continue;
				}

				int dist = static_cast<int>(distanceBetweenUnits(world, attackerId, targetId));
				if (dist >= effectiveMinRange && dist <= effectiveMaxRange)
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

			return std::make_unique<intents::RangedAttackIntent>(attackerId, targetId);
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

			uint32_t damage = ranged.agility;
			if (auto ability = poisonAbilities.find(attackerId); ability != poisonAbilities.end())
			{
				if (dis(gen) <= ability->second.chance)
				{
					world.pushIntent(
							std::make_unique<intents::AddEffectIntent>(
									attackerId, targetId, intents::EffectType::Poison, 5, ability->second.poison));
					world.getEvents().event(world.getTick(), events::UnitAbilityUsed{attackerId, "poison"});
					damage = 0;
				}
			}

			if (damage > 0)
			{
				world.pushIntent(std::make_unique<intents::DamageIntent>(attackerId, targetId, damage, "ranged"));
			}
		}

		static uint32_t distanceBetweenUnits(core::World& world, uint32_t lhsId, uint32_t rhsId)
		{
			auto lhsCells = MarchSystem::getOccupiedCells(world, lhsId);
			auto rhsCells = MarchSystem::getOccupiedCells(world, rhsId);

			if (lhsCells.empty() || rhsCells.empty())
			{
				return 0;
			}

			uint32_t minDistance = std::numeric_limits<uint32_t>::max();
			for (const auto& lhsCell : lhsCells)
			{
				for (const auto& rhsCell : rhsCells)
				{
					minDistance = std::min(minDistance, chebyshevDistance(lhsCell, rhsCell));
				}
			}

			return minDistance;
		}

		static uint32_t chebyshevDistance(core::domain::Position lhs, core::domain::Position rhs)
		{
			auto dx = std::abs(static_cast<int>(lhs.x) - static_cast<int>(rhs.x));
			auto dy = std::abs(static_cast<int>(lhs.y) - static_cast<int>(rhs.y));
			return static_cast<uint32_t>(std::max(dx, dy));
		}
	};
}

#pragma once

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include "Core/World.hpp"
#include "Core/Domain/Position.hpp"
#include "Core/Events/UnitMoved.hpp"

#include "Features/Domain/MarchTarget.hpp"
#include "Features/Domain/PositionOccupier.hpp"
#include "Features/Events/MarchEnded.hpp"
#include "Features/Intents/MarchIntent.hpp"

namespace sw::features::systems::MarchSystem
{
    static bool isPassable(core::World& world, core::domain::Position pos)
    {
        if (pos.x >= world.map.width || pos.y >= world.map.height)
        {
            return false;
        }

        const auto& occupiers = world.getComponent<domain::PositionOccupier>();
        const auto& positions = world.getComponent<core::domain::Position>();

        for (const auto& [id, _] : occupiers)
        {
            auto positionIt = positions.find(id);
            if (positionIt != positions.end() && positionIt->second == pos)
            {
                return false;
            }
        }

        return true;
    }

    static core::domain::Position getNextStep(core::domain::Position current, core::domain::Position target)
    {
        core::domain::Position next = current;

        if (target.x < current.x)      next.x -= 1;
        else if (target.x > current.x) next.x += 1;

        if (target.y < current.y)      next.y -= 1;
        else if (target.y > current.y) next.y += 1;

        return next;
    }

    static void findTargets(core::World& world, uint32_t selfId, core::domain::Position center, std::vector<uint32_t>& out)
    {
        const auto& occupiers = world.getComponent<domain::PositionOccupier>();
        const auto& positions = world.getComponent<core::domain::Position>();

        for (const auto& [id, _] : occupiers)
        {
            if (id == selfId)
            {
                continue;
            }

            auto positionIt = positions.find(id);
            if (positionIt == positions.end())
            {
                continue;
            }

            const auto& pos = positionIt->second;
            if (std::abs(static_cast<int>(pos.x) - static_cast<int>(center.x)) <= 1 &&
                std::abs(static_cast<int>(pos.y) - static_cast<int>(center.y)) <= 1)
            {
                out.push_back(id);
            }
        }
    }

    static std::shared_ptr<intents::MarchIntent> plan(core::World& world, uint32_t unitId)
    {
        auto& targets = world.getComponent<domain::MarchTarget>();
        auto targetIt = targets.find(unitId);
        if (targetIt == targets.end())
        {
            return nullptr;
        }

        auto& positions = world.getComponent<core::domain::Position>();
        auto currentIt = positions.find(unitId);
        if (currentIt == positions.end())
        {
            return nullptr;
        }

        const auto currentPos = currentIt->second;
        const auto nextPos = getNextStep(currentPos, targetIt->second.position);

        if (isPassable(world, nextPos))
        {
            return std::make_shared<intents::MarchIntent>(unitId, currentPos, nextPos);
        }

        return nullptr;
    }

    static void execute(core::World& world, intents::MarchIntent& intent)
    {
        world.getComponent<core::domain::Position>()[intent.unitId] = intent.posTo;
        world.getEvents().event(world.getTick(), core::events::UnitMoved{intent.unitId, intent.posTo.x, intent.posTo.y});
    }

    static void onAfterMove(core::World& world, intents::MarchIntent& intent)
    {
        auto& targets = world.getComponent<domain::MarchTarget>();
        auto it = targets.find(intent.unitId);
        if (it != targets.end() && it->second.position == intent.posTo)
        {
            world.getEvents().event(world.getTick(), features::events::MarchEnded{intent.unitId, intent.posTo.x, intent.posTo.y});
            targets.erase(it);
        }
    }
}

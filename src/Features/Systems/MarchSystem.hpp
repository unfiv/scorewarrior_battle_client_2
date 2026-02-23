#pragma once

#include <algorithm>
#include <cstdint>
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
    static std::vector<core::domain::Position> getOccupiedCells(core::World& world, uint32_t unitId, core::domain::Position anchor)
    {
        std::vector<core::domain::Position> cells;

        const auto& occupiers = world.getComponent<domain::PositionOccupier>();
        auto occupierIt = occupiers.find(unitId);
        if (occupierIt == occupiers.end())
        {
            cells.push_back(anchor);
            return cells;
        }

        for (const auto& offset : occupierIt->second.offsets)
        {
            int64_t cellX = static_cast<int64_t>(anchor.x) + static_cast<int64_t>(offset.x);
            int64_t cellY = static_cast<int64_t>(anchor.y) + static_cast<int64_t>(offset.y);
            if (cellX < 0 || cellY < 0)
            {
                continue;
            }

            cells.push_back({static_cast<uint32_t>(cellX), static_cast<uint32_t>(cellY)});
        }

        if (cells.empty())
        {
            cells.push_back(anchor);
        }

        return cells;
    }

    static std::vector<core::domain::Position> getOccupiedCells(core::World& world, uint32_t unitId)
    {
        const auto& positions = world.getComponent<core::domain::Position>();
        auto positionIt = positions.find(unitId);
        if (positionIt == positions.end())
        {
            return {};
        }

        return getOccupiedCells(world, unitId, positionIt->second);
    }

    static bool isPassable(core::World& world, uint32_t unitId, core::domain::Position pos)
    {
        auto nextCells = getOccupiedCells(world, unitId, pos);
        for (const auto& cell : nextCells)
        {
            if (cell.x >= world.map.width || cell.y >= world.map.height)
            {
                return false;
            }
        }

        const auto& occupiers = world.getComponent<domain::PositionOccupier>();
        for (const auto& [otherId, _] : occupiers)
        {
            if (otherId == unitId)
            {
                continue;
            }

            auto otherCells = getOccupiedCells(world, otherId);
            for (const auto& nextCell : nextCells)
            {
                if (std::find(otherCells.begin(), otherCells.end(), nextCell) != otherCells.end())
                {
                    return false;
                }
            }
        }

        return true;
    }

    static bool areCellsNeighbors(core::domain::Position lhs, core::domain::Position rhs)
    {
        return std::abs(static_cast<int>(lhs.x) - static_cast<int>(rhs.x)) <= 1 &&
               std::abs(static_cast<int>(lhs.y) - static_cast<int>(rhs.y)) <= 1;
    }

    static void findTargets(core::World& world, uint32_t selfId, std::vector<uint32_t>& out)
    {
        const auto& occupiers = world.getComponent<domain::PositionOccupier>();
        auto selfCells = getOccupiedCells(world, selfId);

        for (const auto& [id, _] : occupiers)
        {
            if (id == selfId)
            {
                continue;
            }

            auto targetCells = getOccupiedCells(world, id);
            bool isNeighbor = false;
            for (const auto& selfCell : selfCells)
            {
                if (std::any_of(targetCells.begin(), targetCells.end(), [&](const auto& targetCell)
                {
                    return areCellsNeighbors(selfCell, targetCell);
                }))
                {
                    isNeighbor = true;
                    break;
                }
            }

            if (isNeighbor)
            {
                out.push_back(id);
            }
        }
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
        if (currentPos == targetIt->second.position)
        {
            return nullptr;
        }

        const auto nextPos = getNextStep(currentPos, targetIt->second.position);

        if (isPassable(world, unitId, nextPos))
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

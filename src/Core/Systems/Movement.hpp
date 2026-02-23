#pragma once

#include "Core/World.hpp"
#include "Core/Systems/Spatial.hpp"
#include "Features/Intents/MoveIntent.hpp"

namespace sw::core::systems
{
    class Movement
    {
    public:
        static void processUnit(World& world, uint32_t unitId)
        {
            auto& targets = world.targetPositions;
            if (targets.find(unitId) == targets.end()) return;

            // 1. Check if some Feature-level system (like Battle) blocked us
            if (!world.restrictions.isAllowed(unitId, registry::restrictions::MOVE)) return;

            const Position targetPos = targets[unitId];
            const Position currentPos = world.positions[unitId];
            const Position nextPos = Spatial::getNextStep(currentPos, targetPos);
            if (Spatial::isPassable(world, nextPos))
            {
                world.pushIntent(std::make_shared<features::intents::MoveIntent>(
                    unitId,
                    currentPos.x,
                    currentPos.y,
                    nextPos
                ));
            }
        }
    };
}

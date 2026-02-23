#pragma once

#include "Core/World.hpp"
#include "Core/Events/UnitMoved.hpp"
#include "Core/Events/MarchEnded.hpp"
#include "Features/Intents/MoveIntent.hpp"

namespace sw::features::systems::MovementSystem
{
    // Core logic for updating position
    static void execute(core::World& world, intents::MoveIntent& intent) {
        world.positions[intent.unitId] = intent.posTo;
        world.getEvents().event(world.getTick(), core::events::UnitMoved{intent.unitId, intent.posTo.x, intent.posTo.y});

        /*auto& pos = world.getComponent<core::Position>(intent.unitId);
        // Assuming world.map handles the actual grid logic
        world.map.moveUnit(intent.unitId, pos.x, pos.y, intent.toX, intent.toY);
        pos.x = intent.toX;
        pos.y = intent.toY;*/
    }

    static void onAfterMove(core::World& world, intents::MoveIntent& intent) {
        // Log movement or trigger events here
        auto it = world.targetPositions.find(intent.unitId);
        if (it != world.targetPositions.end() && it->second == intent.posTo) {
            world.getEvents().event(world.getTick(), core::events::MarchEnded{intent.unitId, intent.posTo.x, intent.posTo.y});
            world.targetPositions.erase(it);
        }
    }
}

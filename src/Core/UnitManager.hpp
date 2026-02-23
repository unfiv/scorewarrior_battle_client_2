#pragma once

#include <string>
#include <functional>

#include "Core/World.hpp"
#include "Core/IO/EventSystem.hpp"
#include "Core/Events/UnitSpawned.hpp"
#include "Core/Events/UnitDied.hpp"

namespace sw::core
{
    class UnitManager
    {
    public:
        template<typename F>
        static void spawn(World& world, uint32_t id, const std::string& type, Position pos, F&& setup)
        {
            world.positions[id] = pos;
			world.creationOrder.push_back(id);
            setup();
            world.getEvents().event(world.getTick(), events::UnitSpawned{id, type, pos.x, pos.y});
        }

        template<typename F>
        static void destroy(World& world, uint32_t id, F&& cleanup)
        {
            if (world.positions.erase(id))
            {
                world.targetPositions.erase(id);
                world.intentsChains.erase(id);
                std::erase(world.creationOrder, id);
				cleanup(id);
                world.getEvents().event(world.getTick(), events::UnitDied{id});
            }
        }
    };
}

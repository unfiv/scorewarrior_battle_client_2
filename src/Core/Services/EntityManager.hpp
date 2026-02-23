#pragma once

#include <string>
#include <functional>

#include "Core/World.hpp"
#include "Core/Domain/Position.hpp"
#include "Core/IO/EventSystem.hpp"
#include "Core/Events/UnitSpawned.hpp"
#include "Core/Events/UnitDied.hpp"

namespace sw::core::services
{
    class EntityManager
    {
    public:
        template<typename F>
        static void spawn(World& world, uint32_t id, const std::string& type, domain::Position pos, F&& setup)
        {
            world.getComponent<domain::Position>()[id] = pos;
			world.creationOrder.push_back(id);
            setup();
            world.getEvents().event(world.getTick(), events::UnitSpawned{id, type, pos.x, pos.y});
        }

        template<typename F>
        static void destroy(World& world, uint32_t id, F&& cleanup)
        {
            cleanup(id);
            std::erase(world.creationOrder, id);
            world.intentsChains.erase(id);
		    world.removeAllComponents(id);
            world.getEvents().event(world.getTick(), events::UnitDied{id});
        }
    };
}

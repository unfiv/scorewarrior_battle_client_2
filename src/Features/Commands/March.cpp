#include "March.hpp"

#include "Core/World.hpp"
#include "Core/Commands/CommandRegistry.hpp"
#include "Core/Domain/Position.hpp"
#include "Core/IO/CommandParser.hpp"
#include "Core/IO/EventSystem.hpp"

#include "Features/Domain/MarchTarget.hpp"
#include "Features/Events/MarchStarted.hpp"

REGISTER_COMMAND([](sw::core::World& world, sw::core::io::CommandParser& parser) {
    parser.add<sw::features::commands::March>([&world](auto command)
    {
        auto& positions = world.getComponent<sw::core::domain::Position>();
        auto it = positions.find(command.unitId);
        if (it == positions.end())
        {
            return;
        }

        const auto currentPos = it->second;
        const sw::core::domain::Position targetPos{command.targetX, command.targetY};

        world.getComponent<sw::features::domain::MarchTarget>()[command.unitId] = {targetPos};
        world.getEvents().event(world.getTick(), sw::features::events::MarchStarted{command.unitId, currentPos.x, currentPos.y, targetPos.x, targetPos.y});
    });
})

#pragma once

#include "Core/World.hpp"
#include "Core/IO/CommandParser.hpp"

#include "Core/Events/MapCreated.hpp"

#include "Core/Commands/CommandRegistry.hpp"
#include "Core/Commands/CreateMap.hpp"

namespace sw::core::services
{
    class CommandDispatcher
    {
    public:
        CommandDispatcher(World& world, io::CommandParser& parser)
        {
            parser.add<commands::CreateMap>([&world](auto command)
			{
                world.map = {command.width, command.height};
                world.getEvents().event(world.getTick(), events::MapCreated{command.width, command.height});
            });

            // Setup all the custom commands registered
			commands::CommandRegistry::setupAll(world, parser);
        }
    };
}

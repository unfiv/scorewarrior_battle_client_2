#include "Core/World.hpp"

#include "Core/IO/EventSystem.hpp"
#include "Core/IO/CommandParser.hpp"
#include "Core/Commands/CreateMap.hpp"
#include "Core/Events/MapCreated.hpp"
#include "Core/CommandDispatcher.hpp"
#include "Core/Systems/Movement.hpp"

#include "Features/Systems/RangedAttack.hpp"
#include "Features/Systems/MeleeAttack.hpp"
#include "Features/Systems/Effects.hpp"
#include "Features/Systems/Death.hpp"

#include "Features/Intents/MoveIntent.hpp"
#include "Features/Intents/DamageIntent.hpp"

#include "Features/Systems/MovementSystem.hpp"
#include "Features/Systems/DamageSystem.hpp"

#include <fstream>

int main(int argc, char** argv)
{
	using namespace sw::core;
	using namespace sw::core::io;
	using namespace sw::core::commands;
	using namespace sw::core::events;

	if (argc != 2)
	{
		throw std::runtime_error("Error: No file specified in command line argument");
	}

	std::ifstream file(argv[1]);
	if (!file)
	{
		throw std::runtime_error("Error: File not found - " + std::string(argv[1]));
	}

	EventSystem events;
	World world(events);
	CommandParser parser;

	sw::core::CommandDispatcher dispatcher(world, parser);

	world.systems.push_back(sw::features::systems::Effects::processUnit);
	world.systems.push_back(sw::features::systems::Death::processUnit);
	world.systems.push_back(sw::features::systems::RangedAttack::processUnit);
	world.systems.push_back(sw::features::systems::MeleeAttack::processUnit);
	world.systems.push_back(sw::core::systems::Movement::processUnit);
	world.systems.push_back(sw::features::systems::Death::update);

	using namespace sw::features::systems;
    using namespace sw::features::intents;

    world.resolver.setExecutor<MoveIntent>(MovementSystem::execute);
    world.resolver.subscribe<MoveIntent>(MovementSystem::onAfterMove);

    world.resolver.setExecutor<DamageIntent>(DamageSystem::apply);

	parser.parse(file);

	while (!world.isGameOver())
	{
		world.nextTick();
	}

	return 0;
}

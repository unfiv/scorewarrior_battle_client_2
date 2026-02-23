#include "Core/World.hpp"

#include "Core/IO/EventSystem.hpp"
#include "Core/IO/CommandParser.hpp"
#include "Core/Commands/CreateMap.hpp"
#include "Core/Events/MapCreated.hpp"
#include "Core/Services/CommandDispatcher.hpp"

#include "Features/Systems/RangedAttack.hpp"
#include "Features/Systems/MeleeAttack.hpp"
#include "Features/Systems/Effects.hpp"
#include "Features/Systems/Death.hpp"

#include "Features/Intents/MarchIntent.hpp"
#include "Features/Intents/DamageIntent.hpp"
#include "Features/Intents/RangedAttackIntent.hpp"
#include "Features/Intents/MeleeAttackIntent.hpp"
#include "Features/Intents/EffectsTickIntent.hpp"
#include "Features/Intents/DeathIntent.hpp"

#include "Features/Systems/MarchSystem.hpp"
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

	sw::core::services::CommandDispatcher dispatcher(world, parser);

	using namespace sw::features::systems;
    using namespace sw::features::intents;

	world.resolver.setPlanner<EffectsTickIntent>(Effects::plan);
    world.resolver.setExecutor<EffectsTickIntent>(Effects::execute);

	world.resolver.setPlanner<DeathIntent>(Death::plan);
    world.resolver.setExecutor<DeathIntent>(Death::execute);

	world.resolver.setPlanner<RangedAttackIntent>(RangedAttack::plan);
    world.resolver.setExecutor<RangedAttackIntent>(RangedAttack::execute);

	world.resolver.setPlanner<MeleeAttackIntent>(MeleeAttack::plan);
    world.resolver.setExecutor<MeleeAttackIntent>(MeleeAttack::execute);

	world.resolver.setPlanner<MarchIntent>(MarchSystem::plan);
    world.resolver.setExecutor<MarchIntent>(MarchSystem::execute);
    world.resolver.subscribe<MarchIntent>(MarchSystem::onAfterMove);

    world.resolver.setExecutor<DamageIntent>(DamageSystem::apply);

	parser.parse(file);

	while (!world.isGameOver())
	{
		world.nextTick();
	}

	return 0;
}

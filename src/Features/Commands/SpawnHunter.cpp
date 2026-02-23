#include "SpawnHunter.hpp"

#include "Core/World.hpp"
#include "Core/UnitManager.hpp"
#include "Core/IO/CommandParser.hpp"
#include "Core/Commands/CommandRegistry.hpp"

#include "Core/Domain/Position.hpp"
#include "Features/Domain/MarchTarget.hpp"
#include "Features/Domain/PositionOccupier.hpp"
#include "Features/Domain/Health.hpp"
#include "Features/Domain/Melee.hpp"
#include "Features/Domain/Ranged.hpp"
#include "Features/Domain/PoisonAbility.hpp"
#include "Features/Intents/RangedAttackIntent.hpp"
#include "Features/Intents/MeleeAttackIntent.hpp"
#include "Features/Intents/MarchIntent.hpp"

namespace sw::features::commands
{
    void SpawnHunter::execute(core::World& world) const
    {
        core::UnitManager::spawn(world, unitId, "hunter", {x, y}, [&]()
        {
            world.getComponent<domain::PositionOccupier>()[unitId];
            world.getComponent<domain::MarchTarget>()[unitId] = { {x, y} };
            world.getComponent<domain::Health>()[unitId] = { hp };
            world.getComponent<domain::Melee>()[unitId] = { strength };
            world.getComponent<domain::Ranged>()[unitId] = { agility, range };
            world.getComponent<domain::PoisonAbility>()[unitId] = { chance, poison };

            world.getIntentsChain(unitId)
                .add<intents::RangedAttackIntent>()
                .add<intents::MeleeAttackIntent>()
                .add<intents::MarchIntent>();
        });
    }
}

REGISTER_COMMAND([](sw::core::World& world, sw::core::io::CommandParser& parser) {
    parser.add<sw::features::commands::SpawnHunter>([&world](auto cmd) {
        cmd.execute(world);
    });
})

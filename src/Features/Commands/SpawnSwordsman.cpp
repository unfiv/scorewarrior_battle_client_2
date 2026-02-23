#include "SpawnSwordsman.hpp"

#include "Core/World.hpp"
#include "Core/UnitManager.hpp"
#include "Core/IO/CommandParser.hpp"
#include "Core/Commands/CommandRegistry.hpp"

#include "Features/Domain/Health.hpp"
#include "Features/Domain/Melee.hpp"
#include "Features/Domain/RendingAbility.hpp"
#include "Features/Intents/MeleeAttackIntent.hpp"
#include "Features/Intents/MoveIntent.hpp"

namespace sw::features::commands
{
    void SpawnSwordsman::execute(core::World& world) const
    {
        core::UnitManager::spawn(world, unitId, "swordsman", {x, y}, [&]()
        {
            world.getComponent<domain::Health>()[unitId] = { hp };
            world.getComponent<domain::Melee>()[unitId] = { strength };
            world.getComponent<domain::RendingAbility>()[unitId] = { chance, rending };

            world.getIntentsChain(unitId)
                .add<intents::MeleeAttackIntent>()
                .add<intents::MoveIntent>();
        });
    }
}

REGISTER_COMMAND([](sw::core::World& world, sw::core::io::CommandParser& parser) {
    parser.add<sw::features::commands::SpawnSwordsman>([&world](auto cmd) {
        cmd.execute(world);
    });
})

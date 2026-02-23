#include "SpawnSwordsman.hpp"

#include "Core/World.hpp"
#include "Core/Services/EntityManager.hpp"
#include "Core/IO/CommandParser.hpp"
#include "Core/Commands/CommandRegistry.hpp"

#include "Core/Domain/Position.hpp"
#include "Features/Domain/MarchTarget.hpp"
#include "Features/Domain/PositionOccupier.hpp"
#include "Features/Domain/Health.hpp"
#include "Features/Domain/Melee.hpp"
#include "Features/Domain/MeleeAttackable.hpp"
#include "Features/Domain/RangedAttackable.hpp"
#include "Features/Domain/RendingAbility.hpp"
#include "Features/Intents/MeleeAttackIntent.hpp"
#include "Features/Intents/MarchIntent.hpp"

namespace sw::features::commands
{
    void SpawnSwordsman::execute(core::World& world) const
    {
        core::services::EntityManager::spawn(world, unitId, "swordsman", {x, y}, [&]()
        {
            world.getComponent<domain::PositionOccupier>()[unitId];
            world.getComponent<domain::MarchTarget>()[unitId] = { {x, y} };
            world.getComponent<domain::Health>()[unitId] = { hp };
            world.getComponent<domain::Melee>()[unitId] = { strength };
            world.getComponent<domain::MeleeAttackable>()[unitId];
            world.getComponent<domain::RangedAttackable>()[unitId];
            world.getComponent<domain::RendingAbility>()[unitId] = { chance, rending };

            world.getIntentsChain(unitId)
                .add<intents::MeleeAttackIntent>()
                .add<intents::MarchIntent>();
        });
    }
}

REGISTER_COMMAND([](sw::core::World& world, sw::core::io::CommandParser& parser) {
    parser.add<sw::features::commands::SpawnSwordsman>([&world](auto cmd) {
        cmd.execute(world);
    });
})

#include "World.hpp"

#include <array>
#include <span>

#include "Core/Domain/Position.hpp"

#include "Features/Intents/DeathIntent.hpp"
#include "Features/Intents/EffectsTickIntent.hpp"

namespace sw::core
{
	void World::nextTick()
	{
		++tick;

		restrictions.clear();

		auto ids = creationOrder;
        auto hasPosition = [this](uint32_t id)
        {
            const auto& positions = getComponent<domain::Position>();
            return positions.find(id) != positions.end();
        };

        const std::array<std::type_index, 2> maintenanceChain = {
            std::type_index(typeid(features::intents::EffectsTickIntent)),
            std::type_index(typeid(features::intents::DeathIntent))
        };

        auto executeChain = [this](uint32_t id, std::span<const std::type_index> chain, bool stopOnSuccess)
        {
            for (const auto& intentType : chain)
            {
                auto planner = resolver.getPlanner(intentType);
                if (!planner)
                {
                    continue;
                }

                auto intent = planner(*this, id);
                if (!intent)
                {
                    continue;
                }

                if (resolver.resolve(*this, intent) && stopOnSuccess)
                {
                    break;
                }
            }
        };

        for (uint32_t id : ids)
        {
            // `ids` is a snapshot from the tick start. A unit can die and be removed
            // from components while this tick is still executing, so we must skip stale ids.
            if (!hasPosition(id))
            {
                continue;
            }

            executeChain(id, maintenanceChain, false);

            auto chainIt = intentsChains.find(id);
            if (chainIt == intentsChains.end())
            {
                continue;
            }

            executeChain(id, chainIt->second.get(), true);
        }

        for (uint32_t id : ids)
        {
            if (!hasPosition(id))
            {
                continue;
            }

            executeChain(id, std::span<const std::type_index>(maintenanceChain).subspan(1, 1), false);
        }
	}
}

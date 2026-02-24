#include "World.hpp"

#include "Core/Domain/Position.hpp"

#include <array>
#include <span>
#include <utility>

namespace sw::core
{
	World::World(io::EventSystem& events)
		: eventSystem(events)
	{
	}

	uint32_t World::getTick() const
	{
		return tick;
	}

	void World::nextTick()
	{
		++tick;

		auto ids = creationOrder;
		auto hasPosition = [this](uint32_t id)
		{
			const auto& positions = getComponent<domain::Position>();
			return positions.find(id) != positions.end();
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

				if (resolver.resolve(*this, std::move(intent)) && stopOnSuccess)
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

			executeChain(id, tickSystemOrder, false);

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

			executeChain(id, postTickSystemOrder, false);
		}
	}

	io::EventSystem& World::getEvents()
	{
		return eventSystem;
	}

	bool World::isGameOver()
	{
		auto hasPosition = [this](uint32_t id)
		{
			const auto& positions = getComponent<domain::Position>();
			return positions.find(id) != positions.end();
		};

		auto canPlanAnyIntent = [this](uint32_t id, const std::vector<std::type_index>& chain)
		{
			for (const auto& intentType : chain)
			{
				auto planner = resolver.getPlanner(intentType);
				if (!planner)
				{
					continue;
				}

				if (planner(*this, id))
				{
					return true;
				}
			}

			return false;
		};

		for (uint32_t id : creationOrder)
		{
			if (!hasPosition(id))
			{
				continue;
			}

			if (canPlanAnyIntent(id, tickSystemOrder))
			{
				return false;
			}

			auto chainIt = intentsChains.find(id);
			if (chainIt != intentsChains.end() && canPlanAnyIntent(id, chainIt->second.get()))
			{
				return false;
			}
		}

		return true;
	}

	void World::removeAllComponents(uint32_t id)
	{
		for (auto& clean : componentCleaners)
		{
			clean(id);
		}
	}

	void World::pushIntent(std::unique_ptr<pipeline::Intent> intent)
	{
		resolver.resolve(*this, std::move(intent));
	}
}

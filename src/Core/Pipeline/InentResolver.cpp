#include "IntentResolver.hpp"

#include <cassert>

#include "Core/World.hpp"

namespace sw::core::pipeline
{
	bool IntentResolver::resolve(World& world)
	{
        bool executed = false;
        while (!intentQueue.empty())
        {
            auto current = intentQueue.front();
            intentQueue.pop();

            auto it = registry.find(std::type_index(typeid(*current)));
            if (it == registry.end()) continue;

            auto& hooks = it->second;

            // 1. PRE-PROCESS
            for (auto& hook : hooks.pre) hook(world, *current);
            if (current->getCancelled()) continue;

            // 2. EXECUTE
            assert(hooks.executor);
            hooks.executor(world, *current);
            executed = true;

            // 3. POST-PROCESS
            for (auto& hook : hooks.post) hook(world, *current);
        }

        return executed;
	}

	bool IntentResolver::resolve(World& world, std::shared_ptr<pipeline::Intent> intent)
	{
		emit(intent);
		return resolve(world);
	}
}

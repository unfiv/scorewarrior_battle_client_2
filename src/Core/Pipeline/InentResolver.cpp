#include "IntentResolver.hpp"

#include "Core/World.hpp"

namespace sw::core::pipeline
{
	void IntentResolver::resolve(World& world)
	{
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
            if (hooks.executor) hooks.executor(world, *current);

            // 3. POST-PROCESS
            for (auto& hook : hooks.post) hook(world, *current);
        }
	}

	void IntentResolver::resolve(World& world, std::shared_ptr<pipeline::Intent> intent)
	{
		emit(intent);
		resolve(world);
	}
}

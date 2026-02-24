#pragma once

#include "Core/World.hpp"
#include "Features/Domain/Effects/EffectData.hpp"
#include "Features/Domain/Effects/EffectList.hpp"
#include "Features/Intents/EffectsTickIntent.hpp"

#include <algorithm>

namespace sw::features::systems
{
	class Effects
	{
	public:
		static std::unique_ptr<intents::EffectsTickIntent> plan(core::World& world, uint32_t targetId)
		{
			auto& effectMap = world.getComponent<domain::effects::EffectList>();

			auto itList = effectMap.find(targetId);
			if (itList == effectMap.end() || itList->second.active.empty())
			{
				return nullptr;
			}

			return std::make_unique<intents::EffectsTickIntent>(targetId);
		}

		static void execute(core::World& world, intents::EffectsTickIntent& intent)
		{
			auto targetId = intent.unitId;
			auto& effectMap = world.getComponent<domain::effects::EffectList>();

			auto itList = effectMap.find(targetId);
			if (itList == effectMap.end())
			{
				return;
			}

			auto& effectList = itList->second;
			auto it = effectList.active.begin();

			while (it != effectList.active.end())
			{
				if (it->applyFn)
				{
					it->applyFn(world, targetId, *it);
				}

				if (it->remainingTicks > 0)
				{
					--it->remainingTicks;
				}

				if (it->remainingTicks == 0)
				{
					it = effectList.active.erase(it);
				}
				else
				{
					++it;
				}
			}

			auto& rendingMap = world.getComponent<domain::effects::RendingEffectData>();
			const bool hasRending = std::any_of(
					effectList.active.begin(),
					effectList.active.end(),
					[](const domain::effects::ActiveEffect& effect)
					{ return effect.dataType == std::type_index(typeid(domain::effects::RendingEffectData)); });
			if (!hasRending)
			{
				rendingMap.erase(targetId);
			}
		}
	};
}

#pragma once

#include "Intent.hpp"

#include <functional>
#include <memory>
#include <queue>
#include <typeindex>
#include <unordered_map>

namespace sw::core
{
	class World;
}

namespace sw::core::pipeline
{
	class IntentResolver
	{
		using Handler = std::function<void(World&, pipeline::Intent&)>;
		using Planner = std::function<std::shared_ptr<Intent>(World&, uint32_t)>;

		template <typename T>
		using IntentFunc = std::function<void(World&, T&)>;

		struct Hooks
		{
			Planner planner;
			std::vector<Handler> pre;
			Handler executor;
			std::vector<Handler> post;
		};

		std::unordered_map<std::type_index, Hooks> registry;

	public:
		Planner getPlanner(std::type_index type) const
		{
			auto it = registry.find(type);
			return (it != registry.end()) ? it->second.planner : nullptr;
		}

		bool resolve(World& world);
		bool resolve(World& world, std::shared_ptr<Intent> intent);

		template <typename TIntent>
		void setPlanner(std::function<std::shared_ptr<TIntent>(World&, uint32_t)> func)
		{
			registry[std::type_index(typeid(TIntent))].planner
					= [func](World& w, uint32_t id) -> std::shared_ptr<Intent>
			{
				return func(w, id);
			};
		}

		template <typename TIntent>
		void setExecutor(IntentFunc<TIntent> func)
		{
			registry[std::type_index(typeid(TIntent))].executor = [func](World& w, pipeline::Intent& i) -> void
			{
				func(w, static_cast<TIntent&>(i));
			};
		}

		template <typename TIntent>
		void subscribe(IntentFunc<TIntent> func, bool isPost = true)
		{
			auto& hooks = registry[std::type_index(typeid(TIntent))];
			auto wrapper = [func](World& w, pipeline::Intent& i) -> void
			{
				func(w, static_cast<TIntent&>(i));
			};
			if (isPost)
			{
				hooks.post.push_back(wrapper);
			}
			else
			{
				hooks.pre.push_back(wrapper);
			}
		}
	};
}

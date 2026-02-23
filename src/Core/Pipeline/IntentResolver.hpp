#pragma once

#include <functional>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <queue>

#include "Intent.hpp"

namespace sw::core
{
    class World;
}

namespace sw::core::pipeline
{
    class IntentResolver
    {
        using Handler = std::function<void(World&, pipeline::Intent&)>;

        struct Hooks
        {
            std::vector<Handler> pre;
            Handler executor;
            std::vector<Handler> post;
        };

        std::unordered_map<std::type_index, Hooks> registry;

    public:
        void resolve(World& world);
        void resolve(World& world, std::shared_ptr<Intent> intent);

        template<typename TIntent>
        void setExecutor(std::function<void(World&, TIntent&)> func)
        {
            registry[std::type_index(typeid(TIntent))].executor = 
                [func](World& w, pipeline::Intent& i)
                {
                    func(w, static_cast<TIntent&>(i));
                };
        }

        template<typename TIntent>
        void subscribe(std::function<void(World&, TIntent&)> func, bool isPost = true)
        {
            auto& hooks = registry[std::type_index(typeid(TIntent))];
            auto wrapper = [func](World& w, pipeline::Intent& i)
            {
                func(w, static_cast<TIntent&>(i));
            };
            if (isPost) hooks.post.push_back(wrapper);
            else hooks.pre.push_back(wrapper);
        }

        
        void emit(std::shared_ptr<pipeline::Intent> intent) { intentQueue.push(intent); }

    private:
        // TODO: should we store data?
        std::queue<std::shared_ptr<pipeline::Intent>> intentQueue;
    };
}
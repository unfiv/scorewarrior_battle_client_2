#pragma once

#include <unordered_map>
#include <any>
#include <typeindex>

#include "Core/Domain/Map.hpp"

#include "Core/Pipeline/IntentResolver.hpp"
#include "Core/Pipeline/IntentChain.hpp"

namespace sw::core::io
{
    class EventSystem;
}

namespace sw::core
{
    class World
    {
    public:
        explicit World(io::EventSystem& events) : eventSystem(events) {}

        uint32_t getTick() const { return tick; }
        void nextTick();
        io::EventSystem& getEvents() { return eventSystem; }

        domain::Map map{0, 0};
        pipeline::IntentResolver resolver;

        std::unordered_map<uint32_t, pipeline::IntentChain> intentsChains;

        std::vector<uint32_t> creationOrder;
        

        void pushIntent(std::shared_ptr<pipeline::Intent> intent)
        {
            resolver.resolve(*this, intent);
        }

        template<typename T>
        std::unordered_map<uint32_t, T>& getComponent()
        {
            auto typeIdx = std::type_index(typeid(T));
            auto& storage = components[typeIdx];
            
            if (!storage.has_value())
            {
                storage = std::make_any<std::unordered_map<uint32_t, T>>();

                componentCleaners.push_back([this, typeIdx](uint32_t id)
                {
                    auto it = components.find(typeIdx);
                    if (it != components.end() && it->second.has_value())
                    {
                        auto& table = std::any_cast<std::unordered_map<uint32_t, T>&>(it->second);
                        table.erase(id);
                    }
                });
            }

            return std::any_cast<std::unordered_map<uint32_t, T>&>(storage);
        }

        void removeAllComponents(uint32_t id)
        {
            for (auto& clean : componentCleaners)
            {
                clean(id);
            }
        }

        pipeline::IntentChain& getIntentsChain(uint32_t id)
        {
            return intentsChains[id];
        }

        bool isGameOver() const { return tick > 100; }

    private:
        uint32_t tick{0};
        io::EventSystem& eventSystem;
        std::unordered_map<std::type_index, std::any> components;
        std::vector<std::function<void(uint32_t)>> componentCleaners;
    };
}

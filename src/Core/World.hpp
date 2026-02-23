#pragma once

#include <unordered_map>
#include <any>
#include <typeindex>

#include "Core/Position.hpp"
#include "Core/Map.hpp"
#include "Core/Registry/RestrictionsRegistry.hpp"

#include "Core/Pipeline/IntentResolver.hpp"
#include "Core/Pipeline/IntentChain.hpp"

namespace sw::core::io
{
    class EventSystem;
}

namespace sw::core
{
    class World;

    class World
    {
    public:
        explicit World(io::EventSystem& events) : eventSystem(events) {}

        uint32_t getTick() const { return tick; }
        void nextTick();
        io::EventSystem& getEvents() { return eventSystem; }

        Map map{0, 0};
        pipeline::IntentResolver resolver;

        std::unordered_map<uint32_t, pipeline::IntentChain> intentsChains;

        std::vector<uint32_t> creationOrder;
        std::unordered_map<uint32_t, Position> positions;
        std::unordered_map<uint32_t, Position> targetPositions;
        
        registry::RestrictionsRegistry restrictions;

        void pushIntent(std::shared_ptr<pipeline::Intent> intent)
        {
            resolver.resolve(*this, intent);
        }

        template<typename T>
        std::unordered_map<uint32_t, T>& getComponent()
        {
            auto& storage = components[std::type_index(typeid(T))];
            if (!storage.has_value())
            {
                storage = std::make_any<std::unordered_map<uint32_t, T>>();
            }
            return std::any_cast<std::unordered_map<uint32_t, T>&>(storage);
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
    };
}

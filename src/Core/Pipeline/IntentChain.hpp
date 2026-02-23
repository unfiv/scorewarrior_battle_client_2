#pragma once

#include <typeindex>
#include <vector>

namespace sw::core::pipeline
{
    class IntentChain
    {
    public:
        template<typename TIntent>
        IntentChain& add()
        {
            chain.emplace_back(typeid(TIntent));
            return *this;
        }

        const std::vector<std::type_index>& get() const { return chain; }

    private:
        std::vector<std::type_index> chain;
    };
}

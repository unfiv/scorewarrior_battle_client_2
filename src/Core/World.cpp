#include "World.hpp"

#include <algorithm>

namespace sw::core
{
	void World::nextTick()
	{
		++tick;

		restrictions.clear();

        auto ids = creationOrder;

        for (size_t i = 0; i < systems.size(); ++i)
        {
            if (std::holds_alternative<GlobalSystem>(systems[i]))
            {
                std::get<GlobalSystem>(systems[i])(*this);
            }
            else
            {
                size_t j = i;
                while (j < systems.size() && std::holds_alternative<UnitSystem>(systems[j]))
                {
                    j++;
                }

                for (uint32_t id : ids)
                {
                    for (size_t k = i; k < j; ++k)
                    {
                        if (positions.find(id) == positions.end())
                        {
                            break;
                        }
                        std::get<UnitSystem>(systems[k])(*this, id);
                    }
                }

                i = j - 1;
            }
        }
	}
}

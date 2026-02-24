#pragma once

#include <cstdint>
#include <string>

namespace sw::features::events
{
	struct UnitAbilityUsed
	{
		constexpr static const char* Name = "UNIT_ABILITY_USED";

		uint32_t abilityUnitId{};
		uint32_t targetUnitId{};
		std::string abilityName{};

		template <typename Visitor>
		void visit(Visitor& visitor)
		{
			visitor.visit("abilityUnitId", abilityUnitId);
			visitor.visit("targetUnitId", targetUnitId);
			visitor.visit("abilityName", abilityName);
		}
	};
}

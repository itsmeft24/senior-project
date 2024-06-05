#pragma once
#include "BattleEntity.hpp"
#include "Persona.hpp"

namespace ntr {
	class BattleSystem {
	public:
		bool targets_self(ntr::Skill skill_id);
		bool can_perform_skill(const ntr::BattleEntity* source, ntr::Skill skill_id);
		bool perform_skill(ntr::BattleEntity* source, ntr::BattleEntity* target, ntr::Skill skill_id);
		bool perform_phys(ntr::BattleEntity* source, ntr::BattleEntity* target, int nhits);
	};
};
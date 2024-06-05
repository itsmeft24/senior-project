#include "Persona.hpp"

namespace ntr {
	SkillInfo SKILL_TABLE[] = {
		// Agi is 65, Bufu is 80
		SkillInfo { CostType::SP, SkillType::Fire, 3, 650, "Agi", "Deals weak Fire damage to 1 foe." },
		SkillInfo { CostType::HP, SkillType::Strike, 7, 70, "Bash", "Deals weak Strike damage to 1 foe." },
		SkillInfo { CostType::SP, SkillType::Heal, 3, 90, "Dia", "Slightly restores 1 ally's HP." },
		SkillInfo { CostType::SP, SkillType::StatusDebuff, 8, (STATUS_ATTACK), "Tarunda", "Decreases 1 foe's Attack for 3 turns." },
		SkillInfo { CostType::SP, SkillType::Ice, 4, 80, "Bufu", "Deals weak Ice damage to 1 foe." },
	};

	Skill ORPHEUS_SKILL_IDS[] = {
		Skill::Agi,
		Skill::Bash,
		Skill::Dia,
		Skill::Tarunda
	};
	
	PersonaInfo PERSONA_TABLE[] = {
		PersonaInfo { "Orpheus", 1, 4, ORPHEUS_SKILL_IDS }
	};
};
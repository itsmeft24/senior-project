#pragma once
#include <cstdint>
/*
Orpheus: 
Agi 3 SP	 Deals weak Fire damage to 1 foe.
Dia	3 SP	 Slightly restores 1 ally's HP.
Bash 7% HP	 Deals weak Strike damage to 1 foe.
Tarunda	8 SP Decreases 1 foe's Attack for 3 turns.
*/
namespace ntr {
	constexpr auto MAX_SKILL_COUNT = 5;
	constexpr auto MAX_PERSONA_COUNT = 1;

	constexpr short STATUS_ATTACK = 0x1;
	constexpr short STATUS_DEFENSE = 0x2;
	constexpr short STATUS_ACCURACY = 0x4;
	constexpr short STATUS_EVASION = 0x8;
	// constexpr short STATUS_TURN_COUNT_3 = 0x10;
	
	
	enum CostType {
		SP,
		HP,
		SPPercent,
		HPPercent,
	};

	enum SkillType {
		Strike,
		Fire,
		Ice,
		Heal,
		StatusBuff,
		StatusDebuff,
	};
	
	struct SkillInfo {
		CostType cost_type;
		SkillType skill_type;
		short cost;
		short effect;
		const char* display_name;
		const char* display_description;
	};
	
	extern SkillInfo SKILL_TABLE[MAX_SKILL_COUNT];

	enum class Skill : std::int8_t {
		Agi = 0,
		Bash,
		Dia,
		Tarunda,
		Bufu,
	};

	enum class PersonaID : std::int8_t {
		Orpheus = 0,
	};

	struct PersonaInfo {
		const char* name;
		short level;
		short skill_count;
		Skill* skill_ids;
	};

	extern PersonaInfo PERSONA_TABLE[MAX_SKILL_COUNT];
};
#pragma once
namespace ntr {
	class BattleEntity {
	public:
		short hp;
		short sp;
		signed char attack_lvl;
		signed char defense_lvl;
		signed char accuracy_lvl;
		signed char evasion_lvl;
		signed char status_turn_reset;
	public:
		BattleEntity();
		// must be ran after every turn on all battle entities
		virtual void tick();
	};
};
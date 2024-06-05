#include "BattleEntity.hpp"

namespace ntr {
    BattleEntity::BattleEntity() {
        hp = 0;
        sp = 0;
        attack_lvl=0;
		defense_lvl=0;
		accuracy_lvl=0;
		evasion_lvl=0;
		status_turn_reset=0;
    }
    void BattleEntity::tick() {
        if (status_turn_reset != 0) {
            status_turn_reset--;
            if (status_turn_reset == 0) {
                attack_lvl = 0;
                defense_lvl = 0;
                accuracy_lvl = 0;
                evasion_lvl = 0;
            }
        }
    }
};
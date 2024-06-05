#pragma once
#include <cstdint>
#include "BattleEntity.hpp"
#include "Persona.hpp"

namespace ntr {
	class PersonaUser : public BattleEntity {
	public:
		PersonaInfo* persona;
		// Persona* deck;
		// short persona_count;
	public:
		inline PersonaUser(PersonaID persona_id) : BattleEntity() {
			persona = &PERSONA_TABLE[static_cast<int>(persona_id)];
			hp = 100;
			sp = 50;
		}
	};
};
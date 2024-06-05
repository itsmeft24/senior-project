#include "BattleSystem.hpp"
#include <utility>
#include <cmath>

namespace ntr {
    bool BattleSystem::targets_self(ntr::Skill skill_id) {
        const auto& skill = SKILL_TABLE[std::size_t(skill_id)];
        return skill.skill_type == SkillType::StatusBuff || skill.skill_type == SkillType::Heal;
    }

    bool BattleSystem::can_perform_skill(const ntr::BattleEntity *source, ntr::Skill skill_id)
    {
        const auto& skill = SKILL_TABLE[std::size_t(skill_id)];
        // subtract hp/sp from source
        switch (skill.cost_type) {
            case CostType::HP:
                if (source->hp <= skill.cost) {
                    return false;
                }
                break;
            case CostType::SP:
                if (source->sp <= skill.cost) {
                    return false;
                }
                break;
            case CostType::SPPercent:
            {
                float percentage = (1.0 - static_cast<float>(skill.cost) / 100.0f);
                short new_sp = std::roundf(static_cast<float>(source->sp) * percentage);
                if (new_sp == source->sp || new_sp == 0) {
                    return false;
                }
            }
            break;
            case CostType::HPPercent:
            {
                float percentage = (1.0 - static_cast<float>(skill.cost) / 100.0f);
                short new_hp = std::roundf(static_cast<float>(source->hp) * percentage);
                if (new_hp == source->hp || new_hp == 0) {
                    return false;
                }
            }
            break;
        }
        return true;
    }

    bool BattleSystem::perform_skill(ntr::BattleEntity* source, ntr::BattleEntity* target, ntr::Skill skill_id) {
        const auto& skill = SKILL_TABLE[std::size_t(skill_id)];
        // subtract hp/sp from source
        switch (skill.cost_type) {
            case CostType::HP:
                if (source->hp <= skill.cost) {
                    return false;
                }
                source->hp -= skill.cost;
                break;
            case CostType::SP:
                if (source->sp <= skill.cost) {
                    return false;
                }
                source->sp -= skill.cost;
                break;
            case CostType::SPPercent:
            {
                float percentage = (1.0 - static_cast<float>(skill.cost) / 100.0f);
                short new_sp = std::roundf(static_cast<float>(source->sp) * percentage);
                if (new_sp == source->sp || new_sp == 0) {
                    return false;
                }
                source->sp *= percentage;
            }
            break;
            case CostType::HPPercent:
            {
                float percentage = (1.0 - static_cast<float>(skill.cost) / 100.0f);
                short new_hp = std::roundf(static_cast<float>(source->hp) * percentage);
                if (new_hp == source->hp || new_hp == 0) {
                    return false;
                }
                source->hp *= percentage;
            }
            break;
        }

        if (skill.skill_type == SkillType::StatusBuff) {
            if (skill.effect & STATUS_ATTACK) target->attack_lvl++;
            if (skill.effect & STATUS_DEFENSE) target->defense_lvl++;
            if (skill.effect & STATUS_ACCURACY) target->accuracy_lvl++;
            if (skill.effect & STATUS_EVASION) target->evasion_lvl++;
            target->status_turn_reset = 3;
        }
        
        if (skill.skill_type == SkillType::StatusDebuff) {
            if (skill.effect & STATUS_ATTACK) target->attack_lvl--;
            if (skill.effect & STATUS_DEFENSE) target->defense_lvl--;
            if (skill.effect & STATUS_ACCURACY) target->accuracy_lvl--;
            if (skill.effect & STATUS_EVASION) target->evasion_lvl--;
            target->status_turn_reset = 3;
        }
        
        if (skill.skill_type == SkillType::Fire || skill.skill_type == SkillType::Strike || skill.skill_type == SkillType::Ice) {
            if (source->attack_lvl > 0) {
                target->hp -= std::sqrt(static_cast<float>(skill.effect) * float(source->attack_lvl + 1));
            } else if (source->attack_lvl < 0) {
                target->hp -= std::sqrt(static_cast<float>(skill.effect) / float(1 - source->attack_lvl));
            } else {
                target->hp -= std::sqrt(static_cast<float>(skill.effect));
            }
        }

        if (skill.skill_type == SkillType::Heal) {
            target->hp += std::sqrt(static_cast<float>(skill.effect));
        }

        return true;
    }
    bool BattleSystem::perform_phys(ntr::BattleEntity *source, ntr::BattleEntity *target, int nhits)
    {
        if (source->attack_lvl > 0) {
            target->hp -= std::sqrt(7.5f * float(nhits) * float(source->attack_lvl + 1));
        } else if (source->attack_lvl < 0) {
            target->hp -= std::sqrt(7.5f * float(nhits) / float(1 - source->attack_lvl));
        } else {
            target->hp -= std::sqrt(7.5f * float(nhits));
        }
        return true;
    }
};
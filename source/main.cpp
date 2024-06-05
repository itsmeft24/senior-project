#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <grrlib.h>
#include <fat.h>
#include <sdcard/wiisd_io.h>
#include <ogcsys.h>

#include "ntr/gfx/backend_temp/Defs.hpp"
#include "ntr/gfx/backend_temp/Renderer.hpp"
#include "ntr/gfx/backend_temp/Texture.hpp"
#include "ntr/gfx/backend_temp/Model.hpp"
#include "ntr/gfx/backend_temp/AnimationDataFile.hpp"
#include "ntr/AnimationSet.hpp"
#include "ntr/GenericProgressBar.hpp"
#include "ntr/AudioSystem.hpp"
#include "ntr/AudioFile.hpp"

#include "ntr/logic/BattleSystem.hpp"
#include "ntr/logic/PersonaUser.hpp"
#include <unistd.h>
#include <ogc/lwp_watchdog.h>

std::size_t num_used = 0;

const quat DEFAULT_CAMERA_ROTATION{ 0.073758, 0.772638, -0.628461, -0.051259 };
const vec3 DEFAULT_CAMERA_POSITION{-260.218, -578.972, -255.263};

const vec2 HP_METER_DEFAULT_VERTICES[] = {
	vec2{2237.0f * (672.0f / 2560.0f) - 34.0f, 1532.0f * (480.0f / 1829.0f)},
	vec2{2365.0f * (672.0f / 2560.0f) - 34.0f, 1484.0f * (480.0f / 1829.0f)},
	vec2{2365.0f * (672.0f / 2560.0f) - 34.0f, 1495.0f * (480.0f / 1829.0f)},
	vec2{2237.0f * (672.0f / 2560.0f) - 34.0f, 1544.0f * (480.0f / 1829.0f)},
};

const vec2 SP_METER_DEFAULT_VERTICES[] = {
	vec2{(2237.0f + 20.0f) * (672.0f / 2560.0f) - 34.0f, (1532.0f + 10.0f) * (480.0f / 1829.0f)},
	vec2{(2365.0f + 20.0f) * (672.0f / 2560.0f) - 34.0f, (1484.0f + 10.0f) * (480.0f / 1829.0f)},
	vec2{(2365.0f + 20.0f) * (672.0f / 2560.0f) - 34.0f, (1495.0f + 10.0f) * (480.0f / 1829.0f)},
	vec2{(2237.0f + 20.0f) * (672.0f / 2560.0f) - 34.0f, (1544.0f + 10.0f) * (480.0f / 1829.0f)},
};

const vec2 PARTY_PANEL_BACKING_VERTICES[] = {
	vec2{2219.0f * (672.0f / 2560.0f) - 34.0f, 1532.0f * (480.0f / 1829.0f)},
	vec2{2485.0f * (672.0f / 2560.0f) - 34.0f, 1430.0f * (480.0f / 1829.0f)},
	vec2{2485.0f * (672.0f / 2560.0f) - 34.0f, 1477.0f * (480.0f / 1829.0f)},
	vec2{2238.0f * (672.0f / 2560.0f) - 34.0f, 1573.0f * (480.0f / 1829.0f)},
};

const vec2 ENEMY_SELECT_HP_DEFAULT_VERTICES[] = {
	vec2{37, 24},
	vec2{87, 24},
	vec2{87, 29},
	vec2{37, 29},
};

const vec2 PLAYER_SELECT_HP_DEFAULT_VERTICES[] = {
	vec2{33, 25},
	vec2{82, 25},
	vec2{82, 30},
	vec2{33, 30},
};

enum GameState {
	CommandMenu,
	PersonaMenu,
	TargetSelectPersonaUsers,
	TargetSelectEnemies,
	SummonPersona,
	ShowSkillAction,
	SummonPersonaCriticalHit,
	ShowAttackAction,
	ShowSelfStatusChange,
	ShowEnemyStatusChange,
	ShowEnemyMagicAttack,
	Victory,
};

using ntr::Skill;

int main( int argc, char **argv ){
	// 13, 192
	// 117, 301
	SYS_STDIO_Report(true);

	fatInitDefault();
	fatMountSimple("sd", &__io_wiisd);

	WPAD_Init();

	ntr::AudioSystem audio{};
	ntr::gfx::Renderer renderer{};
	ntr::gfx::Camera camera{};

	camera.position = DEFAULT_CAMERA_POSITION;
	camera.rotation = DEFAULT_CAMERA_ROTATION;

	ntr::gfx::Model player_model{};
	ntr::gfx::Model persona_model{};
	ntr::gfx::Model enemy_model{};
	ntr::gfx::Model floor_model{};
	ntr::AnimationSet player_model_anim_set{};
	ntr::AnimationSet persona_model_anim_set{};
	ntr::AnimationSet enemy_model_anim_set{};

	printf("Loading models...\n");
	player_model.load("sd:/nitrogen/player/makoto/makoto_full_skinned.mdl", "sd:/nitrogen/player/makoto");
	player_model.set_scale({-0.973328,0.973328,-0.973328});
	persona_model.load("sd:/nitrogen/persona/orpheus/orpheus_full_skinned.mdl", "sd:/nitrogen/persona/orpheus");
	enemy_model.load("sd:/nitrogen/enemy/maya/maya_full_skinned_minified.mdl", "sd:/nitrogen/enemy/maya");
	enemy_model.set_scale({0.7,0.7,0.7});
	floor_model.load("sd:/nitrogen/map/floor.mdl", "sd:/nitrogen/map");
	floor_model.set_position({-100, 500, 25});
	floor_model.set_rotation({0.1, -2.6, 92.5});
	
	
	printf("Loading animations...\n");
	player_model_anim_set.add_animation("sd:/nitrogen/player/makoto/anim/draw_evoker.anm", false, -1); // 0
	player_model_anim_set.add_animation("sd:/nitrogen/player/makoto/anim/hold_evoker.anm", true, -1); // 1
	player_model_anim_set.add_animation("sd:/nitrogen/player/makoto/anim/retract_evoker.anm", false, -1); // 2
	player_model_anim_set.add_animation("sd:/nitrogen/player/makoto/anim/idle_c_start.anm", false, -1); // 3
	player_model_anim_set.add_animation("sd:/nitrogen/player/makoto/anim/idle_c.anm", true, -1); // 4
	player_model_anim_set.add_animation("sd:/nitrogen/player/makoto/anim/summon_act_start.anm", false, -1); // 5
	player_model_anim_set.add_animation("sd:/nitrogen/player/makoto/anim/summon_act_loop.anm", true, -1); // 6
	player_model_anim_set.add_animation("sd:/nitrogen/player/makoto/anim/summon_act_end.anm", false, -1); // 7
	player_model_anim_set.add_animation("sd:/nitrogen/player/makoto/anim/attack_a.anm", false, -1); // 8
	player_model_anim_set.add_animation("sd:/nitrogen/player/makoto/anim/damage.anm", false, -1); // 9
	player_model_anim_set.add_animation("sd:/nitrogen/player/makoto/anim/victory.anm", false, -1); // 10
	player_model_anim_set.set_index(3);

	enemy_model_anim_set.add_animation("sd:/nitrogen/enemy/maya/anim/idle.anm", true, -1);
	enemy_model_anim_set.set_index(0);
	
	persona_model_anim_set.add_animation("sd:/nitrogen/persona/orpheus/anim/idle.anm", false, -1);
	persona_model_anim_set.add_animation("sd:/nitrogen/persona/orpheus/anim/attack_phys.anm", false, -1);
	persona_model_anim_set.add_animation("sd:/nitrogen/persona/orpheus/anim/attack_magic_start.anm", false, -1);
	persona_model_anim_set.add_animation("sd:/nitrogen/persona/orpheus/anim/attack_magic_loop.anm", true, -1);
	persona_model_anim_set.add_animation("sd:/nitrogen/persona/orpheus/anim/attack_magic_end.anm", false, -1);
	persona_model_anim_set.set_index(0);

	printf("Loading UI textures...\n");
	ntr::gfx::Texture tarot_card{"sd:/nitrogen/ui/tarot_card.tex0\0"};
	ntr::gfx::Texture persona_text_underbar{"sd:/nitrogen/ui/persona_text_underbar.tex0\0"};
	ntr::gfx::Texture persona_text{"sd:/nitrogen/ui/persona_text.tex0\0"};
	ntr::gfx::Texture persona_label{"sd:/nitrogen/ui/persona_label.tex0\0"};
	ntr::gfx::Texture skill_select_text{"sd:/nitrogen/ui/skill_select_text.tex0\0"};
	
	ntr::gfx::Texture command_moon{"sd:/nitrogen/ui/command_moon.tex0\0"};
	ntr::gfx::Texture button_circle{"sd:/nitrogen/ui/button_circle.tex0\0"};
	
	ntr::gfx::Texture agi_black{"sd:/nitrogen/ui/persona_menu/agi_blk.tex0"};
	ntr::gfx::Texture agi_blue{"sd:/nitrogen/ui/persona_menu/agi_blu.tex0"};
	ntr::gfx::Texture bash_black{"sd:/nitrogen/ui/persona_menu/bash_blk.tex0"};
	ntr::gfx::Texture bash_blue{"sd:/nitrogen/ui/persona_menu/bash_blu.tex0"};
	ntr::gfx::Texture dia_black{"sd:/nitrogen/ui/persona_menu/dia_blk.tex0"};
	ntr::gfx::Texture dia_blue{"sd:/nitrogen/ui/persona_menu/dia_blu.tex0"};
	ntr::gfx::Texture tarunda_black{"sd:/nitrogen/ui/persona_menu/tarunda_blk.tex0"};
	ntr::gfx::Texture tarunda_blue{"sd:/nitrogen/ui/persona_menu/tarunda_blu.tex0"};
	ntr::gfx::Texture select_bar{"sd:/nitrogen/ui/persona_menu/skill_sel.tex0"};
	ntr::gfx::Texture skill_table_view{"sd:/nitrogen/ui/persona_menu/skill_set.tex0"};
	ntr::gfx::Texture party_panel_makoto{"sd:/nitrogen/ui/party_panel_makoto.tex0"};
	ntr::gfx::Texture target_select_enemy{"sd:/nitrogen/ui/target_select_enemy.tex0"};
	ntr::gfx::Texture enemy_label{"sd:/nitrogen/ui/enemy_label.tex0"};
	ntr::gfx::Texture target_select_player{"sd:/nitrogen/ui/target_select_player.tex0"};
	ntr::gfx::Texture player_label{"sd:/nitrogen/ui/player_label.tex0"};
	ntr::gfx::Texture stat_arrow{"sd:/nitrogen/ui/stat_arrow.tex0"};

	printf("Loading sound effects...\n");

	ntr::AudioFile bgm{"sd:/nitrogen/music/bgm_ambush_min.wav"};
	ntr::AudioFile win{"sd:/nitrogen/music/bgm_win.wav"};
	ntr::AudioFile battle_start{"sd:/nitrogen/audio/sfx/battle_start.wav"};
	ntr::AudioFile draw_evoker{"sd:/nitrogen/audio/sfx/draw_evoker.wav"};
	ntr::AudioFile evoker_shoot_loud{"sd:/nitrogen/audio/sfx/evoker_shoot_loud.wav"};
	ntr::AudioFile evoker_shoot_soft{"sd:/nitrogen/audio/sfx/evoker_shoot_soft.wav"};
	ntr::AudioFile open_cmd_menu{"sd:/nitrogen/audio/sfx/open_cmd_menu.wav"};
	ntr::AudioFile summon_start_sfx{"sd:/nitrogen/audio/sfx/summon_start.wav"};
	ntr::AudioFile system_cursor{"sd:/nitrogen/audio/sfx/system_cursor.wav"};
	// ntr::AudioFile system_ok{"sd:/nitrogen/audio/sfx/system_ok.wav"};
	ntr::AudioFile target_select{"sd:/nitrogen/audio/sfx/target_select.wav"};
	ntr::AudioFile win_snd{"sd:/nitrogen/audio/vox/win_06.wav"};

	ntr::AudioFile summon_start[] = {
		{"sd:/nitrogen/audio/vox/summon_start_1.wav"},
		{"sd:/nitrogen/audio/vox/summon_start_2.wav"},
		{"sd:/nitrogen/audio/vox/summon_start_3.wav"}
	};

	ntr::AudioFile callout[] = {
		{"sd:/nitrogen/audio/vox/callout_1.wav"},
		{"sd:/nitrogen/audio/vox/callout_2.wav"},
		{"sd:/nitrogen/audio/vox/callout_persona_orpheus.wav"},
		{"sd:/nitrogen/audio/vox/callout_3.wav"}	
	};
	
	printf("Initializing battle system...\n");
	ntr::PersonaUser player{ntr::PersonaID::Orpheus};
	ntr::BattleEntity enemy{};
	ntr::BattleSystem system{};
	enemy.hp = 50;
	enemy.sp = 1000;

	bool x_down = false;
	GameState state = GameState::CommandMenu;
	GameState prev_state = GameState::CommandMenu;
	// bool in_persona_menu = false;
	Skill selected_skill = Skill::Agi;
	bool incoming_player_turn_is_phys = false;

	float state_timer = -1;
	bool is_persona_visible = false;
	bool bingus = false;
	float delta_time = 0.01f;
	// ntr::GenericProgressBar hp{vec2{50,50}, 30.0f, 20.0f, 0xFFFF00FF};

	ntr::GenericProgressBar hp_meter_back{
		(vec2*)&HP_METER_DEFAULT_VERTICES,
		GXColor{0x02, 0x5a, 0xb5, 0xff}
	};
	ntr::GenericProgressBar hp_meter_bar{
		(vec2*)&HP_METER_DEFAULT_VERTICES,
		GXColor{0x31, 0xe2, 0xfb, 0xff}
	};
	ntr::GenericProgressBar sp_meter_back{
		(vec2*)&SP_METER_DEFAULT_VERTICES,
		GXColor{0x56, 0x47, 0x1d, 0xff}
	};
	ntr::GenericProgressBar sp_meter_bar{
		(vec2*)&SP_METER_DEFAULT_VERTICES,
		GXColor{0xfe, 0xe1, 0x31, 0xff}
	};
	ntr::GenericProgressBar party_panel_backing{
		(vec2*)&PARTY_PANEL_BACKING_VERTICES,
		GXColor{0x00, 0x00, 0x00, 0xff}
	};

	ntr::GenericProgressBar enemy_select_hp_backing{
		(vec2*)&ENEMY_SELECT_HP_DEFAULT_VERTICES,
		GXColor{0x09, 0x6e, 0x6e, 0xff}
	};

	ntr::GenericProgressBar player_select_hp_backing{
		(vec2*)&PLAYER_SELECT_HP_DEFAULT_VERTICES,
		GXColor{0x09, 0x6e, 0x6e, 0xff}
	};

	ntr::GenericProgressBar enemy_select_hp{
		(vec2*)&ENEMY_SELECT_HP_DEFAULT_VERTICES,
		GXColor{0x3d, 0xf2, 0xfc, 0xff}
	};

	ntr::GenericProgressBar player_select_hp{
		(vec2*)&PLAYER_SELECT_HP_DEFAULT_VERTICES,
		GXColor{0x3d, 0xf2, 0xfc, 0xff}
	};
	
	audio.play_music(bgm, true, 1000);
	audio.play_ui_sound_effect(battle_start);

	while (!renderer.is_window_closed()) {
		// printf("USED HEAP: %d", num_used);
		auto before = gettime();
		WPAD_ScanPads();

		if ((WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_HOME) || (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)) {
			// printf("Exiting...");
			renderer.set_window_closed();
		}

		if (state == GameState::CommandMenu) {
			if (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_B) {
				audio.play_ui_sound_effect(target_select);
				state = GameState::TargetSelectEnemies;
				incoming_player_turn_is_phys = true;
				state_timer = 0;
			} else if (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_X) {
				player_model_anim_set.set_index(0);
				state = GameState::PersonaMenu;
				audio.play_sound_effect(summon_start[rand() % 3]);
				audio.play_ui_sound_effect(draw_evoker);
			}
		} else if (state == GameState::PersonaMenu) {
			if (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_A) {
				if (system.can_perform_skill(&player, selected_skill)) {
					incoming_player_turn_is_phys = false;
					audio.play_ui_sound_effect(target_select);
					state = system.targets_self(selected_skill) ? GameState::TargetSelectPersonaUsers : GameState::TargetSelectEnemies;
					state_timer = 0;
				}
			} else if ((WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_B) || (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_X)) {
				player_model_anim_set.set_index(2);
				state = GameState::CommandMenu;
				audio.play_ui_sound_effect(open_cmd_menu);
			} else if (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_DOWN) {
				selected_skill = (Skill)((int)selected_skill + 1);
				if ((int)selected_skill == 4) {
					selected_skill = Skill::Agi;
				}
				audio.play_ui_sound_effect(system_cursor);
				// printf("Selected Skill: %d\n", (int)selected_skill);
			} else if (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_UP) {
				selected_skill = (Skill)((int)selected_skill - 1);
				if ((int)selected_skill == -1) {
					selected_skill = Skill::Tarunda;
				}
				audio.play_ui_sound_effect(system_cursor);
				// printf("Selected Skill: %d\n", (int)selected_skill);
			}
		}

		// Enemy Model
		enemy_model.set_position({-110, 870, 0});
		enemy_model.set_rotation({-90,0,0});

		// Persona Model
		persona_model.set_position(vec3{-1.45, -67.016, -133.16});
		persona_model.set_rotation({-90,0,0});
		persona_model.set_scale(vec3{0.87,0.87,-0.87});
		
		// Player Model
		player_model.set_scale({-1,1,-1});
		//player_model.set_position(vec3{-0.02,-0.73,0});
		// player_model.set_rotation(vec3{180.0f+1.80f,1,-4.9+180});
		player_model.set_rotation(vec3{-90.0f + 1.80f,1,-4.9});
		if (state == GameState::TargetSelectEnemies) {
			camera.rotation = ntr::util::slerp_quat(ntr::util::clamp((state_timer / 6.0f) * (state_timer / 6.0f), 0.0f, 1.0f), DEFAULT_CAMERA_ROTATION, quat{0.058912, 0.773911, -0.629329, -0.039185});

			if (state_timer != 0.0f && (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_A)) {
				if (incoming_player_turn_is_phys) {
					state = GameState::ShowAttackAction;
					state_timer = -delta_time * 30.0f;
				}
				else {
					state = GameState::SummonPersona;
					state_timer = -delta_time * 30.0f;
				}
			}
			if (state_timer != 0.0f && (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_B)) {
				state = incoming_player_turn_is_phys ? GameState::CommandMenu : GameState::PersonaMenu;
				camera.rotation = DEFAULT_CAMERA_ROTATION;
				camera.position = DEFAULT_CAMERA_POSITION;
				state_timer = 0.0f;
			}
			state_timer += delta_time * 30.0f;
		}

		if (state == GameState::TargetSelectPersonaUsers) {
			float t_in = (1.0f - std::cos(3.14159265358979f / 2.0f * state_timer / 34.0f)) / 2.0f;
			// float t_in = ntr::util::clamp((state_timer / 34.0f) * (state_timer / 34.0f), 0.0f, 1.0f);
			camera.position = ntr::util::lerp_vec(t_in, vec3{-126.991, 631.47, -140.703}, vec3{-153.681, 631.47, -142.223});
			camera.rotation = ntr::util::slerp_quat(t_in, quat{-0.720774, -0.071308, 0.067881, 0.686144}, quat{-0.719197, -0.085759, 0.081639, 0.684643});

			if (state_timer != 0.0f && (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_A)) {
				state = GameState::SummonPersona;
				state_timer = -delta_time * 30.0f;
			}
			if ((WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_B)) {
				state = GameState::PersonaMenu;
				camera.rotation = DEFAULT_CAMERA_ROTATION;
				camera.position = DEFAULT_CAMERA_POSITION;
				state_timer = 0.0f;
			}
			state_timer += delta_time * 30.0f;
		}

		if (state == GameState::SummonPersona) {
			if (state_timer == 0) {
				audio.play_ui_sound_effect(summon_start_sfx);
				player_model.set_position(vec3{-0.02,-0.73,0});
				persona_model_anim_set.set_index(0);
				player_model_anim_set.set_index(5);
			}
			if (state_timer >= 0.1 && !bingus) {
				bingus = true;
				audio.play_sound_effect(callout[rand() % 4]);
				if (rand() % 2 == 0) {
					audio.play_ui_sound_effect(evoker_shoot_soft);
				} else {
					audio.play_ui_sound_effect(evoker_shoot_loud);
				}
			}
			if (player_model_anim_set.get_index() == 5 && player_model_anim_set.current_finished()) {
				player_model_anim_set.set_index(6);
			}
			if (state_timer >= 0 && state_timer < 9) {
				camera.position = vec3{-0.246927,217.968,-104.865};
				camera.rotation = quat{0.684674,0,0,-0.728849};
			}
			if (state_timer >= 9 && state_timer < 16) {
				camera.position = ntr::util::lerp_vec(((float)state_timer - 9.0f) / (16.0f - 9.0f), vec3{-0.246927,217.968,-104.865}, vec3{-2.31593, 381.838, -95.545});
			}
			if (state_timer >= 20) {
				is_persona_visible = true;
			}
			if (state_timer >= 16 && state_timer < 27) {
				
				camera.position = vec3{-2.31593, 381.838, -95.545};
			}
			if (state_timer >= 27 && state_timer < 59) {
				persona_model_anim_set.set_index(2);
				camera.position = ntr::util::lerp_vec(((float)state_timer - 27.0f) / (59.0f - 27.0f), vec3{121.046,324.293,-99.635}, vec3{200.78,428.72,-85.81});
				camera.rotation = ntr::util::slerp_quat(((float)state_timer - 27.0f) / (59.0f - 27.0f), quat{0.617581, -0.114462, 0.141803, -0.765104}, quat{0.55494, -0.102852, 0.150437, -0.811685});
			}
			state_timer += 30.0f * delta_time;
			if (state_timer >= 78) {
				state_timer = 0;
				bingus = false;
				if (system.targets_self(selected_skill)) {
					is_persona_visible = false;
					state = GameState::ShowSelfStatusChange;
				} else {
					if (ntr::SKILL_TABLE[(int)selected_skill].skill_type == ntr::SkillType::StatusDebuff) {
						state = GameState::ShowEnemyStatusChange;
					} else {
						state = GameState::ShowSkillAction;
					}
				}
			}
		}

		if (state == GameState::ShowSkillAction) {
			if (persona_model_anim_set.get_index() == 2 && persona_model_anim_set.current_finished()) {
				persona_model_anim_set.set_index(3);
			}
			if (state_timer <= 85) {
				camera.position = ntr::util::lerp_vec(((float)state_timer) / 85.0f, vec3{-437.648,-945.092,-407.403}, vec3{-434.058,-950.116,-414.263});
				camera.rotation = ntr::util::slerp_quat(((float)state_timer) / 85.0f, quat{0.105352, 0.770081, -0.623934, -0.081126}, quat{0.108711, 0.769614, -0.623575, -0.083848});
			}
			state_timer += 30.0f * delta_time;
			if (state_timer >= 92.0f && state_timer < 92.4f) {
				player_model_anim_set.set_index(7);
				persona_model_anim_set.set_index(4);
			}
			if (state_timer >= 102) {
				state_timer = 0;
				/*
				player_model_anim_set.set_index(3);
				camera.position = DEFAULT_CAMERA_POSITION;
				camera.rotation = DEFAULT_CAMERA_ROTATION;
				*/
				is_persona_visible = false;
				system.perform_skill(&player, &enemy, selected_skill);
				enemy.tick();
				state = GameState::ShowEnemyMagicAttack;
				// player.tick();
				// state = GameState::CommandMenu;
				// audio.play_ui_sound_effect(open_cmd_menu);
			}
		}

		if (state == GameState::ShowEnemyStatusChange) {
			if (persona_model_anim_set.get_index() == 2 && persona_model_anim_set.current_finished()) {
				persona_model_anim_set.set_index(3);
			}
			if (state_timer <= 57) {
				camera.position = ntr::util::lerp_vec(state_timer / 57.0f, vec3{-244.265,1450,-219.986}, vec3{-262.765,1504.38,-218.096});
				camera.rotation = ntr::util::slerp_quat(state_timer / 57.0f, quat{-0.712164, -0.131992, 0.12565, 0.677948}, quat{-0.706817, -0.131001, 0.126683, 0.68352});
			}
			state_timer += 30.0f * delta_time;
			if (state_timer >= 80.0f && state_timer < 80.4f) {
				player_model_anim_set.set_index(7);
				persona_model_anim_set.set_index(4);
			}
			if (state_timer >= 83) {
				state_timer = 0;
				/*
				player_model_anim_set.set_index(3);
				camera.position = DEFAULT_CAMERA_POSITION;
				camera.rotation = DEFAULT_CAMERA_ROTATION;
				*/
				is_persona_visible = false;
				system.perform_skill(&player, &enemy, selected_skill);
				enemy.tick();
				state = GameState::ShowEnemyMagicAttack;
				// player.tick();
				// state = GameState::CommandMenu;
				// audio.play_ui_sound_effect(open_cmd_menu);
			}
		}

		if (state == GameState::ShowSelfStatusChange) {
			if (player_model_anim_set.get_index() == 6 && player_model_anim_set.current_finished()) {
				player_model_anim_set.set_index(7);
			}
			float t_in = ntr::util::clamp((state_timer / 45.0f) * (state_timer / 45.0f), 0.0f, 1.0f);
			camera.position = ntr::util::lerp_vec(t_in, vec3{-126.991, 631.47, -140.703}, vec3{-153.681, 631.47, -142.223});
			camera.rotation = ntr::util::slerp_quat(t_in, quat{-0.720774, -0.071308, 0.067881, 0.686144}, quat{-0.719197, -0.085759, 0.081639, 0.684643});

			state_timer += delta_time * 30.0f;
			if (state_timer >= 45.0f) {
				state_timer = 0;
				/*
				player_model_anim_set.set_index(3);
				camera.position = DEFAULT_CAMERA_POSITION;
				camera.rotation = DEFAULT_CAMERA_ROTATION;
				*/
				is_persona_visible = false;
				system.perform_skill(&player, &player, selected_skill);
				enemy.tick();
				state = GameState::ShowEnemyMagicAttack;
				// player.tick();
				// state = GameState::CommandMenu;
				// audio.play_ui_sound_effect(open_cmd_menu);
			}
		}

		if (state == GameState::ShowAttackAction) {
			player_model.set_rotation(vec3{180.0f + 1.80f,1,-4.9 + 180.0f});
			enemy_model.set_position(vec3{-13.82, 467.12, 0.0f});
			if (state_timer == 0) {
				player_model_anim_set.set_index(8);
			}
			if (state_timer >= 0 && state_timer < 52) {
				camera.position = ntr::util::lerp_vec((float)state_timer / 33.0f, vec3{-219.806,-425.4,-223.482}, vec3{-241.536,-425.4,-224.442});
				camera.rotation = ntr::util::slerp_quat((float)state_timer / 33.0f, quat{-0.108711, -0.769614, 0.623575, 0.083848}, quat{-0.108039, -0.769709, 0.623648, 0.083304});
			}
			if (state_timer >= 32 && state_timer < 52) {
				
				player_model.position.y = ntr::util::lerp((state_timer - 32.0f) / 8.0f, 0.0f, -200.0f);

				if (state_timer <= 36) {
					float t_in = ((state_timer - 32.0f) / 4.0f);
					player_model.position.z = ntr::util::lerp(t_in * t_in, 0.0f, -10.0f);
				} else {
					float t_in = ((state_timer - 36.0f) / 4.0f);
					player_model.position.z = ntr::util::lerp(t_in * t_in, -10.0f, 20.0f);
				}
			}
			state_timer += 30.0f * delta_time;
			if (state_timer >= 52) {
				state_timer = 0;
				/*
				player_model_anim_set.set_index(3);
				camera.position = DEFAULT_CAMERA_POSITION;
				camera.rotation = DEFAULT_CAMERA_ROTATION;
				*/
				system.perform_phys(&player, &enemy, 1);
				enemy.tick();
				state = GameState::ShowEnemyMagicAttack;
				// player.tick();
				// state = GameState::CommandMenu;
				// audio.play_ui_sound_effect(open_cmd_menu);
			}
		}

		if (enemy.hp <= 0 && state != GameState::Victory) {
			enemy.hp = 0;
			state = GameState::Victory;
			state_timer = 0;
		}
		/*
		WOW THE ENEMY CAN ATTACK NOW!
		*/
		if (state == GameState::ShowEnemyMagicAttack) {
			player_model.set_position(vec3{0,0,0});
			if (state_timer == 0) player_model_anim_set.set_index(4);
			if (state_timer <= 28) {
				camera.position = ntr::util::lerp_vec(state_timer / 28.0f, vec3{-214.923,-503.679,-147.375}, vec3{-226.313,-541.119,-115.995});
				camera.rotation = ntr::util::slerp_quat(state_timer / 28.0f, quat{-0.113259, -0.71049, 0.685873, 0.109336}, quat{-0.11075, -0.694747, 0.701815, 0.111877});
			}
			state_timer += 30.0f * delta_time;
			if (state_timer >= 45.0f) {
				if (state_timer < 45.4f) {
					player_model_anim_set.set_index(9);
				}
				player_model.set_rotation(vec3{180.0f + 1.80f,1,-4.9 + 180.0f});
			}
			if (state_timer >= 92) {
				state_timer = 0;
				
				player_model_anim_set.set_index(3);
				camera.position = DEFAULT_CAMERA_POSITION;
				camera.rotation = DEFAULT_CAMERA_ROTATION;
				
				system.perform_skill(&enemy, &player, Skill::Bufu);
				player.tick();
				state = GameState::CommandMenu;
				audio.play_ui_sound_effect(open_cmd_menu);
			}
		}

		if (state == GameState::Victory) {
			camera.position.x = 1.537;
			camera.rotation = quat{0.705536, 0.047103, 0.047103, -0.705536};
			player_model.set_position(vec3{0,0,0});
			player_model.set_rotation(vec3{180.0f,0,180.0f});
			if (state_timer == 0) {
				player_model_anim_set.set_index(10);
				audio.play_sound_effect(win_snd);
			}
			
			if (state_timer < 31.0f) {
				camera.position.z = ntr::util::lerp(state_timer / 31.0f, -153.92f, -151.876f);
			} else if (state_timer >= 31.0f && state_timer < 38.0f) {
				camera.position.z = ntr::util::lerp(((state_timer - 31.0f) / 7.0f) * ((state_timer - 31.0f) / 7.0f), -151.876f, -145.922f);
			} else if (state_timer >= 38.0f && state_timer < 45.0f) {
				camera.position.z = ntr::util::lerp((state_timer - 38.0f) / 7.0f, -145.922f, -120.579f);
			} else if (state_timer >= 45.0f && state_timer < 65.0f) {
				camera.position.z = ntr::util::lerp(((state_timer - 45.0f) / 20.0f) * ((state_timer - 45.0f) / 20.0f), -120.579f, -107.483f);
			}
			const float THATS_A_WIN_FOR_THE_TEAM_Y_POS[] = {
				301.711,
				313.72,
				329.986,
				353.395,
				388.385,
				438.543,
				500.689,
				566.833,
				620.271,
				673.708,
				727.146,
				780.583,
				834.021,
				887.458,
				918.193,
				945.857,
				970.612,
				992.619,
				1012.04,
				1029.04,
				1043.77,
				1056.41,
				1067.1,
				1076.02,
				1083.32,
				1089.16,
				1093.71,
				1097.13,
				1099.59,
				1101.23,
				1102.23,
				1102.74,
				1102.93,
				1102.95
			};
			if (state_timer < 29.0f) {
				camera.position.y = ntr::util::lerp(state_timer / 29.0f, 64.678f, 269.362f);
			} else if (state_timer >= 29.0f && state_timer < 31.0f) {
				camera.position.y = ntr::util::lerp((state_timer - 29.0f) / 2.0f, 269.362f, 292.31f);
			} else if (state_timer >= 31.0f && state_timer < 64.0f) {
				float start = THATS_A_WIN_FOR_THE_TEAM_Y_POS[int(state_timer) - 31];
				float end = THATS_A_WIN_FOR_THE_TEAM_Y_POS[int(state_timer) - 30];
				camera.position.y = ntr::util::lerp(state_timer - float(int(state_timer)), start, end);
			}

			state_timer += 30.0f * delta_time;
			
			if (state_timer >= 65.0f && !bingus) {
				camera.position.y = THATS_A_WIN_FOR_THE_TEAM_Y_POS[33];
				audio.play_music(win, true, 0);
				bingus = true;
			}
		}
		
		camera.update();
		renderer.start_3d();
		player_model_anim_set.animate_model(player_model);
		if (is_persona_visible) {
			persona_model_anim_set.animate_model(persona_model);
		}
		enemy_model_anim_set.animate_model(enemy_model);

		
		// Floor Model
		floor_model.draw(camera, renderer, ntr::gfx::DrawType::Simple);
		
		if (state == GameState::CommandMenu) {
			renderer.start_2d();
			command_moon.draw(renderer, vec2{13,192});
			renderer.start_3d();
		}

		// Persona Model
		if (is_persona_visible) {
			persona_model.draw(camera, renderer, ntr::gfx::DrawType::Simple);
		}
		// Enemy Model
		if (state != GameState::Victory) {
			enemy_model.draw(camera, renderer, ntr::gfx::DrawType::Simple);
		}
		if (state == GameState::TargetSelectEnemies) {
			vec2 __dpos = camera.world_to_screen(enemy_model.position);
			renderer.start_2d();
			target_select_enemy.draw(renderer, vec2{__dpos.x - 30, __dpos.y - 90});

			enemy_select_hp_backing.resource_bind();
			enemy_select_hp_backing.set_offset(vec2{__dpos.x - 30, __dpos.y - 90});
			enemy_select_hp_backing.draw(1.0f);

			enemy_select_hp.resource_bind();
			enemy_select_hp.set_offset(vec2{__dpos.x - 30, __dpos.y - 90});
			enemy_select_hp.draw(ntr::util::clamp(float(enemy.hp) / 50.0f, 0.0f, 1.0f));

			enemy_label.draw(renderer, vec2{15, 68});

			if (enemy.attack_lvl != 0) {
				if (enemy.attack_lvl < 0) {
					for (int level = 0; level < std::abs(enemy.attack_lvl); level++) {
						stat_arrow.draw_flipped(renderer, vec2{28.0f, 98.0f + level * 18.0f});
					}
				} else {
					for (int level = 0; level < std::abs(enemy.attack_lvl); level++) {
						stat_arrow.draw(renderer, vec2{28.0f, 98.0f + level * 18.0f});
					}
				}
			}
			if (enemy.defense_lvl != 0) {
				if (enemy.defense_lvl < 0) {
					for (int level = 0; level < std::abs(enemy.defense_lvl); level++) {
						stat_arrow.draw_flipped(renderer, vec2{82.0f, 98.0f + level * 18.0f});
					}
				} else {
					for (int level = 0; level < std::abs(enemy.defense_lvl); level++) {
						stat_arrow.draw(renderer, vec2{82.0f, 98.0f + level * 18.0f});
					}
				}
			}

			renderer.start_3d();
		}
		if (state == GameState::CommandMenu || state == GameState::TargetSelectEnemies) {
			// Player Model Blue Outline
			player_model.set_position(vec3{9.86835,15.2956,1.52247});
			player_model.draw(camera, renderer, ntr::gfx::DrawType::BlueBorder);
			player_model.set_position(vec3{-0.02,-0.73,0});
		}
		// Player Model
		if (state == GameState::CommandMenu || state == GameState::PersonaMenu || state == GameState::TargetSelectPersonaUsers || state == GameState::TargetSelectEnemies) {
			player_model.set_position(vec3{-0.02,-0.73,0});
		}
		player_model.draw(camera, renderer, ntr::gfx::DrawType::Simple);
		if (state == GameState::TargetSelectPersonaUsers) {
			vec2 __dpos = camera.world_to_screen(player_model.position);
			renderer.start_2d();
			target_select_player.draw(renderer, vec2{__dpos.x - 20, __dpos.y + 100});
		
			player_select_hp_backing.resource_bind();
			player_select_hp_backing.set_offset(vec2{__dpos.x - 20, __dpos.y + 100});
			player_select_hp_backing.draw(1.0f);

			player_select_hp.resource_bind();
			player_select_hp.set_offset(vec2{__dpos.x - 20, __dpos.y + 100});
			player_select_hp.draw(ntr::util::clamp(float(player.hp) / 100.0f, 0.0f, 1.0f));

			player_label.draw(renderer, vec2{15, 68});

			// player cannot be debuffed by the enemy, nor can they buff themselves so this UI element will never render anyway
			/*
			if (player.attack_lvl != 0) {
				if (player.attack_lvl < 0) {
					for (int level = 0; level < std::abs(player.attack_lvl); level++) {
						stat_arrow.draw_flipped(renderer, vec2{28.0f, 98.0f + level * 18.0f});
					}
				} else {
					for (int level = 0; level < std::abs(player.attack_lvl); level++) {
						stat_arrow.draw(renderer, vec2{28.0f, 98.0f + level * 18.0f});
					}
				}
			}
			if (player.defense_lvl != 0) {
				if (player.defense_lvl < 0) {
					for (int level = 0; level < std::abs(player.defense_lvl); level++) {
						stat_arrow.draw_flipped(renderer, vec2{82.0f, 98.0f + level * 18.0f});
					}
				} else {
					for (int level = 0; level < std::abs(player.defense_lvl); level++) {
						stat_arrow.draw(renderer, vec2{82.0f, 98.0f + level * 18.0f});
					}
				}
			}
			*/
			renderer.start_3d();
		}
		if (state == GameState::PersonaMenu && ((player_model_anim_set.get_index() == 0 && player_model_anim_set.get_time() >= 0.3f) || player_model_anim_set.get_index() != 0)) {
			renderer.start_2d();
			tarot_card.draw(renderer, vec2{0,46});
			persona_text_underbar.draw(renderer, vec2{0,232});
			persona_text.draw(renderer, vec2{1,208});
			renderer.start_3d();
		}

		// UI Model
		if (state == GameState::PersonaMenu) {
			if (player_model_anim_set.get_index() == 0) {
				player_model.set_scale({1,-1,-1});
				vec3 target_position{-136.261f ,-435.192f ,-106.031f};
				vec3 target_rotation{-90.0f + 17.7147, 180.0f - 36.8658, 126.549};
				player_model.set_position(ntr::util::lerp_vec(std::sqrt(ntr::util::clamp(player_model_anim_set.get_time() * 3.0f, 0.0f, 1.0f)), vec3{}, target_position));
				player_model.set_rotation(ntr::util::lerp_vec(std::sqrt(ntr::util::clamp(player_model_anim_set.get_time() * 3.0f, 0.0f, 1.0f)), vec3{-90.0f,180.0f,0.0f}, target_rotation));
				player_model.draw(camera, renderer, ntr::gfx::DrawType::UITexture);
			}
			if (player_model_anim_set.get_index() == 1) {
				player_model.set_scale({1,-1,-1});
				player_model.set_position(vec3 {-136.261f ,-435.192f ,-106.031f});
				player_model.set_rotation(vec3{-90.0f + 17.7147, 180.0f - 36.8658, 126.549});
				player_model.draw(camera, renderer, ntr::gfx::DrawType::UITexture);
			}
			if (player_model_anim_set.get_index() == 2 && player_model_anim_set.get_time() <= 0.30f) {
				player_model.set_scale({1,-1,-1});
				vec3 target_rotation{-90.0f,180.0f,0.0f};
				player_model.set_position(ntr::util::lerp_vec(std::sqrt(ntr::util::clamp(player_model_anim_set.get_time() * 3.0f, 0.0f, 1.0f)), vec3{-136.261f ,-435.192f ,-106.031f}, vec3{}));
				player_model.set_rotation(ntr::util::lerp_vec(std::sqrt(ntr::util::clamp(player_model_anim_set.get_time() * 3.0f, 0.0f, 1.0f)), vec3{-90.0f + 17.7147, 180.0f - 36.8658, 126.549}, target_rotation));
				player_model.draw(camera, renderer, ntr::gfx::DrawType::UITexture);
			}
		}

		if (state == GameState::PersonaMenu && ((player_model_anim_set.get_index() == 0 && player_model_anim_set.get_time() >= 0.30f) || player_model_anim_set.get_index() != 0)) {
			renderer.start_2d();
			skill_select_text.draw(renderer, vec2{109,378});
			persona_label.draw(renderer, vec2{0,56});
			select_bar.draw(renderer, vec2{151.0f, 122.8f + (18.3f * static_cast<float>(selected_skill))});
			skill_table_view.draw(renderer, vec2{157, 130});

			if (selected_skill == Skill::Agi) {
				agi_black.draw(renderer, vec2{204,131});
			} else {
				agi_blue.draw(renderer, vec2{204,131});
			}
			if (selected_skill == Skill::Bash) {
				bash_black.draw(renderer, vec2{205,149});
			} else {
				bash_blue.draw(renderer, vec2{205,149});
			}
			if (selected_skill == Skill::Dia) {
				dia_black.draw(renderer, vec2{205,168});
			} else {
				dia_blue.draw(renderer, vec2{205,168});
			}
			if (selected_skill == Skill::Tarunda) {
				tarunda_black.draw(renderer, vec2{207,186});
			} else {
				tarunda_blue.draw(renderer, vec2{207,186});
			}
		}

		if (state == GameState::CommandMenu) {
			renderer.start_2d();
			button_circle.draw(renderer, vec2{117,301});
		}

		renderer.start_2d();
		
		party_panel_makoto.draw(renderer, vec2{584 - 32, 344});
		party_panel_backing.resource_bind();
		party_panel_backing.draw(1.0f);
		hp_meter_back.resource_bind();
		hp_meter_back.draw(1.0f);
		hp_meter_bar.resource_bind();
		hp_meter_bar.draw(ntr::util::clamp(float(player.hp) / 100.0f, 0.0f, 1.0f));
		sp_meter_back.resource_bind();
		sp_meter_back.draw(1.0f);
		sp_meter_bar.resource_bind();
		sp_meter_bar.draw(ntr::util::clamp(float(player.sp) / 50.0f, 0.0f, 1.0f));

		renderer.end_frame();

		player_model_anim_set.tick(delta_time);
		persona_model_anim_set.tick(delta_time);
		enemy_model_anim_set.tick(delta_time);
		if (player_model_anim_set.get_index() == 0 && player_model_anim_set.current_finished()) {
			player_model_anim_set.set_index(1);
		}
		if (player_model_anim_set.get_index() == 2 && player_model_anim_set.current_finished()) {
			player_model_anim_set.set_index(4);
		}
		if (player_model_anim_set.get_index() == 3 && player_model_anim_set.current_finished()) {
			player_model_anim_set.set_index(4);
		}
		if (prev_state != state) {
			// printf("%d\n", (int)state);
		}
		prev_state = state;
		delta_time = diff_usec(before, gettime()) / 1.0E6f;
		//printf("%f\n", delta_time);
	}
	// printf("Unmounting...");
	fatUnmount("sd");
	__io_wiisd.shutdown();
	return 0;
}
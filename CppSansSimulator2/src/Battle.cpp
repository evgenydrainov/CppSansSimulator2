#include "Battle.h"

#include "Game.h"

#include "misc.h"
#include <math.h>
#include "mathh.h"

#include <fstream>
#include <sstream>

#define HEART_JUMP_STRENGTH 180.0f
#define HEART_JUMPHOLD_CUTOFF 30.0f
#define HEART_SPEED 150.0f
#define MAX_FALL_SPEED 750.0f
#define BORDER_THICK 3.0f

#define ATTACKS_DIR "attacks/"

static const char* attack_list[] = {
	ATTACKS_DIR "sans_intro.csv",
	ATTACKS_DIR "sans_bluebone.csv",
	ATTACKS_DIR "sans_bonegap1.csv",
	ATTACKS_DIR "sans_bonegap1fast.csv",
	ATTACKS_DIR "sans_boneslideh.csv",
	ATTACKS_DIR "sans_boneslidev.csv",
	ATTACKS_DIR "sans_platforms1.csv",
	ATTACKS_DIR "sans_platforms2.csv",
	ATTACKS_DIR "sans_platforms4.csv",
	ATTACKS_DIR "sans_platforms4hard.csv"
};

static void LoadAttack(const std::string& fname, Attack* attack) {
	std::ifstream f(fname);
	std::string line;
	attack->clear();
	while (std::getline(f, line)) {
		if (line.empty()) continue;

		AttackAction& action = attack->emplace_back();

		size_t end = line.find(',');
		action.delay = std::stof(line.substr(0, end));

		size_t start = end + 1;
		end = line.find(',', start);
		action.func_name = line.substr(start, end - start);

		while (true) {
			start = end + 1;
			end = line.find(',', start);
			if (end == std::string::npos) {
				std::string arg = line.substr(start);
				if (!arg.empty()) action.args.push_back(arg);
				break;
			} else {
				std::string arg = line.substr(start, end - start);
				if (arg.empty()) break;
				action.args.push_back(arg);
			}
		}
	}
}

void Battle::Init() {
	LoadAttack(attack_list[0], &attack);
}

void Battle::Quit() {
	
}

void Battle::Update(float delta) {
	const Uint8* key = SDL_GetKeyboardState(nullptr);

	switch (player.mode) {
		case PlayerMode::RED: {
			float x = (float) (key[SDL_SCANCODE_RIGHT] - key[SDL_SCANCODE_LEFT]);
			float y = (float) (key[SDL_SCANCODE_DOWN]  - key[SDL_SCANCODE_UP]);
			normalize0(x, y, &x, &y);

			player.hsp = HEART_SPEED * x;
			player.vsp = HEART_SPEED * y;
			break;
		}

		case PlayerMode::BLUE: {
			float x = (float) (key[SDL_SCANCODE_RIGHT] - key[SDL_SCANCODE_LEFT]);
			player.hsp = HEART_SPEED * x;
			if (player.y == border_y2 - 8.0f - BORDER_THICK) {
				if (key[SDL_SCANCODE_UP]) {
					player.vsp = -HEART_JUMP_STRENGTH;
				}
			}

			float gravity = 540.0f;
			if (key[SDL_SCANCODE_UP] && player.vsp < 0.0f) {
				gravity = 180.0f;
			}
			if (!key[SDL_SCANCODE_UP]) {
				player.vsp = max(player.vsp, -HEART_JUMPHOLD_CUTOFF);
			}

			player.vsp += gravity * delta;
			player.vsp = min(player.vsp, MAX_FALL_SPEED);
			break;
		}
	}

	player.x += player.hsp * delta;
	player.y += player.vsp * delta;

	player.x = clamp(player.x,
					 border_x1 + 8.0f + BORDER_THICK,
					 border_x2 - 8.0f - BORDER_THICK);
	player.y = clamp(player.y,
					 border_y1 + 8.0f + BORDER_THICK,
					 border_y2 - 8.0f - BORDER_THICK);

	for (auto bone = bones.begin(); bone != bones.end();) {
		switch (bone->dir) {
			case 0: {
				bone->x += bone->speed * delta;
				if (bone->x > border_x2) {
					bone = bones.erase(bone);
					continue;
				}
				break;
			}
			case 1: {
				bone->y += bone->speed * delta;
				if (bone->y > border_y2) {
					bone = bones.erase(bone);
					continue;
				}
				break;
			}
			case 2: {
				bone->x -= bone->speed * delta;
				if (bone->x + bone->width < border_x1) {
					bone = bones.erase(bone);
					continue;
				}
				break;
			}
			case 3: {
				bone->y -= bone->speed * delta;
				if (bone->y + bone->height < border_y1) {
					bone = bones.erase(bone);
					continue;
				}
				break;
			}
		}
		++bone;
	}

	for (auto blaster = blasters.begin(); blaster != blasters.end();) {
		if (blaster->timer > blaster->spin_time + blaster->blast_time) {
			blaster = blasters.erase(blaster);
			continue;
		}

		if (blaster->timer >= blaster->spin_time) {
			blaster->x = blaster->end_x;
			blaster->y = blaster->end_y;
			blaster->angle = blaster->end_angle;
		} else {
			float f = blaster->timer / blaster->spin_time;
			blaster->x = lerp(blaster->start_x, blaster->end_x, f);
			blaster->y = lerp(blaster->start_y, blaster->end_y, f);
			blaster->angle = lerp(0.0f, blaster->end_angle, f);
		}

		blaster->timer += delta;
		++blaster;
	}

	if (bone_stab) {
		if (bone_stab_warn_time > 0.0f) {
			bone_stab_warn_time = max(bone_stab_warn_time - delta, 0.0f);
		} else if (bone_stab_stay_time > 0.0f) {
			bone_stab_stay_time = max(bone_stab_stay_time - delta, 0.0f);
		} else {
			bone_stab = false;
		}
	}

	if (border_x1 != border_x1_target
		|| border_y1 != border_y1_target
		|| border_x2 != border_x2_target
		|| border_y2 != border_y2_target) {
		border_x1 = approach(border_x1, border_x1_target, border_speed * delta);
		border_y1 = approach(border_y1, border_y1_target, border_speed * delta);
		border_x2 = approach(border_x2, border_x2_target, border_speed * delta);
		border_y2 = approach(border_y2, border_y2_target, border_speed * delta);
		if (border_x1 == border_x1_target && border_y1 == border_y1_target && border_x2 == border_x2_target && border_y2 == border_y2_target) {
			if (border_finish_action == "TLResume") {
				attack_paused = false;
			}
		}
	}

	if (!attack_paused) {
		if (attack_action_index >= attack.size()) {

		} else {
			if (attack_wait_timer > 0.0f) {
				attack_wait_timer = std::max(attack_wait_timer - delta, 0.0f);
			} else {
				while (true) {
					AttackAction& current_action = attack[attack_action_index];
					if (current_action.func_name == "CombatZoneResizeInstant") {
						size_t i = 0;
						border_x1 = std::stof(current_action.args[i++]);
						border_y1 = std::stof(current_action.args[i++]);
						border_x2 = std::stof(current_action.args[i++]);
						border_y2 = std::stof(current_action.args[i++]);
						border_x1_target = border_x1;
						border_y1_target = border_y1;
						border_x2_target = border_x2;
						border_y2_target = border_y2;
					} else if (current_action.func_name == "HeartTeleport") {
						size_t i = 0;
						player.x = std::stof(current_action.args[i++]);
						player.y = std::stof(current_action.args[i++]);
					} else if (current_action.func_name == "HeartMode") {
						size_t i = 0;
						player.mode = (PlayerMode) std::stoi(current_action.args[i++]);
					} else if (current_action.func_name == "SansSlam") {
						size_t i = 0;
						int dir = std::stoi(current_action.args[i++]);
						if (dir == 1) {
							player.y = border_y2 - 8.0f;
						}
						player.mode = PlayerMode::BLUE;
					} else if (current_action.func_name == "BoneStab") {
						size_t i = 0;
						bone_stab_dir = std::stoi(current_action.args[i++]);
						bone_stab_dist = std::stof(current_action.args[i++]);
						bone_stab_warn_time = std::stof(current_action.args[i++]);
						bone_stab_stay_time = std::stof(current_action.args[i++]);
						bone_stab = true;
					} else if (current_action.func_name == "GasterBlaster") {
						GasterBlaster& blaster = blasters.emplace_back();

						size_t i = 0;
						blaster.size = std::stof(current_action.args[i++]);
						blaster.start_x = std::stof(current_action.args[i++]);
						blaster.start_y = std::stof(current_action.args[i++]);
						blaster.end_x = std::stof(current_action.args[i++]);
						blaster.end_y = std::stof(current_action.args[i++]);
						blaster.end_angle = -1.0f * std::stof(current_action.args[i++]);
						blaster.spin_time = std::stof(current_action.args[i++]);
						blaster.blast_time = std::stof(current_action.args[i++]);
						blaster.x = blaster.start_x;
						blaster.y = blaster.start_y;
					} else if (current_action.func_name == "BoneH") {
						Bone& bone = bones.emplace_back();

						size_t i = 0;
						bone.x = std::stof(current_action.args[i++]);
						bone.y = std::stof(current_action.args[i++]);
						bone.width = std::stof(current_action.args[i++]);
						bone.height = 10.0f;
						bone.dir = std::stoi(current_action.args[i++]);
						bone.speed = std::stof(current_action.args[i++]);
						bone.color = std::stoi(current_action.args[i++]);
						bone.orient = 0;
					} else if (current_action.func_name == "BoneV") {
						Bone& bone = bones.emplace_back();

						bone.x = std::stof(current_action.args[0]);
						bone.y = std::stof(current_action.args[1]);
						bone.width = 10.0f;
						bone.height = std::stof(current_action.args[2]);
						bone.dir = std::stoi(current_action.args[3]);
						bone.speed = std::stof(current_action.args[4]);
						if (current_action.args.size() > 5) bone.color = std::stoi(current_action.args[5]);
						bone.orient = 1;
					} else if (current_action.func_name == "BoneHRepeat") {
						size_t i = 0;
						float x = std::stof(current_action.args[i++]);
						float y = std::stof(current_action.args[i++]);
						float width = std::stof(current_action.args[i++]);
						int dir = std::stoi(current_action.args[i++]);
						float speed = std::stof(current_action.args[i++]);
						int count = std::stoi(current_action.args[i++]);
						float spacing = std::stof(current_action.args[i++]);

						for (int i = 0; i < count; i++) {
							Bone& bone = bones.emplace_back();
							bone.x = x;
							if (dir == 1) {
								bone.y = y - spacing * (float)i;
							} else if (dir == 3) {
								bone.y = y + spacing * (float)i;
							} else {
								// assert(false);
							}
							bone.width = width;
							bone.height = 10.0f;
							bone.dir = dir;
							bone.speed = speed;
							bone.orient = 0;
						}
					} else if (current_action.func_name == "BoneVRepeat") {
						size_t i = 0;
						float x = std::stof(current_action.args[i++]);
						float y = std::stof(current_action.args[i++]);
						float height = std::stof(current_action.args[i++]);
						int dir = std::stoi(current_action.args[i++]);
						float speed = std::stof(current_action.args[i++]);
						int count = std::stoi(current_action.args[i++]);
						float spacing = std::stof(current_action.args[i++]);

						for (int i = 0; i < count; i++) {
							Bone& bone = bones.emplace_back();
							if (dir == 2) {
								bone.x = x + spacing * (float)i;
								bone.y = y;
							} else if (dir == 0) {
								bone.x = x - spacing * (float)i;
								bone.y = y;
							} else if (dir == 1) {
								bone.x = x;
								bone.y = y - spacing * (float)i;
							} else if (dir == 3) {
								bone.x = x;
								bone.y = y + spacing * (float)i;
							}
							bone.width = 10.0f;
							bone.height = height;
							bone.dir = dir;
							bone.speed = speed;
							bone.orient = 1;
						}
					} else if (current_action.func_name == "CombatZoneResize") {
						size_t i = 0;
						border_x1_target = std::stof(current_action.args[i++]);
						border_y1_target = std::stof(current_action.args[i++]);
						border_x2_target = std::stof(current_action.args[i++]);
						border_y2_target = std::stof(current_action.args[i++]);
						border_finish_action = current_action.args[i++];
					} else if (current_action.func_name == "TLPause") {
						attack_paused = true;
					} else if (current_action.func_name == "EndAttack") {
						player.x = 320.0f;
						player.y = 304.0f;
						player.hsp = 0.0f;
						player.vsp = 0.0f;
						player.mode = PlayerMode::RED;

						border_x1 = 33.0f;
						border_y1 = 251.0f;
						border_x2 = 608.0f;
						border_y2 = 391.0f;
						border_x1_target = 33.0f;
						border_y1_target = 251.0f;
						border_x2_target = 608.0f;
						border_y2_target = 391.0f;
						border_speed = 480.0f;
						border_finish_action.clear();

						bones.clear();
						blasters.clear();

						clipping = true;

						if (attack_index + 1 >= ArrayLength(attack_list)) {
							attack_paused = true;
						} else {
							attack_index++;
							attack_action_index = 0;
							LoadAttack(attack_list[attack_index], &attack);
						}
						break;
					} else if (current_action.func_name == "EnableClipping") {
						clipping = std::stoi(current_action.args[0]);
					}

					attack_action_index++;

					if (attack_action_index >= attack.size()) {
						break;
					}

					float delay = attack[attack_action_index].delay;
					if (delay > 0.0f) {
						attack_wait_timer = delay;
						break;
					}

					if (attack_paused) {
						break;
					}
				}
			}
		}
	}
}

static void draw_bone_v(float x, float y, float height, SDL_Color c = COLOR_WHITE) {
	SDL_Rect top_src = {0, 0, 10, 6};
	SDL_FRect top_dest = {x, y, 10.0f, 6.0f};
	DrawSprite(&game->spr_bone_v, &top_src, &top_dest, c);

	SDL_Rect middle_src = {0, 6, 10, 12};
	SDL_FRect middle_dest = {x, y + 6.0f, 10.0f, height - 12.0f};
	DrawSprite(&game->spr_bone_v, &middle_src, &middle_dest, c);

	SDL_Rect bottom_src = {0, 18, 10, 6};
	SDL_FRect bottom_dest = {x, y + height - 6.0f, 10.0f, 6.0f};
	DrawSprite(&game->spr_bone_v, &bottom_src, &bottom_dest, c);
}

void Battle::Draw(float delta) {
	{
		SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);

		float x = border_x1;
		float y = border_y1;
		float w = border_x2 - border_x1;
		float h = border_y2 - border_y1;

		SDL_FRect top = {x, y, w, 3.0f};
		SDL_RenderFillRectF(game->renderer, &top);
		SDL_FRect left = {x, y, 3.0f, h};
		SDL_RenderFillRectF(game->renderer, &left);
		SDL_FRect bottom = {x, y + h - 3.0f, w, 3.0f};
		SDL_RenderFillRectF(game->renderer, &bottom);
		SDL_FRect right = {x + w - 3.0f, y, 3.0f, h};
		SDL_RenderFillRectF(game->renderer, &right);
	}

	if (clipping) {
		SDL_Rect view;
		view.x = (int) (border_x1 + BORDER_THICK);
		view.y = (int) (border_y1 + BORDER_THICK);
		view.w = (int) (border_x2 - border_x1 - BORDER_THICK * 2.0f);
		view.h = (int) (border_y2 - border_y1 - BORDER_THICK * 2.0f);
		SDL_RenderSetClipRect(game->renderer, &view);
	}

	{
		SDL_Color c = (player.mode == PlayerMode::RED) ? COLOR_RED : COLOR_BLUE;
		DrawSprite(&game->spr_player_heart, 0, player.x, player.y, 0.0f, 1.0f, 1.0f, c);
	}

	for (Bone& bone : bones) {
		if (bone.orient == 0) {
			SDL_Color c = (bone.color == 1) ? SDL_Color{128, 128, 255, 255} : COLOR_WHITE;

			SDL_Rect left_src = {0, 0, 6, 10};
			SDL_FRect left_dest = {bone.x, bone.y, 6.0f, 10.0f};
			DrawSprite(&game->spr_bone_h, &left_src, &left_dest, c);

			SDL_Rect middle_src = {6, 0, 12, 10};
			SDL_FRect middle_dest = {bone.x + 6.0f, bone.y, bone.width - 12.0f, 10.0f};
			DrawSprite(&game->spr_bone_h, &middle_src, &middle_dest, c);

			SDL_Rect right_src = {18, 0, 6, 10};
			SDL_FRect right_dest = {bone.x + bone.width - 6.0f, bone.y, 6.0f, 10.0f};
			DrawSprite(&game->spr_bone_h, &right_src, &right_dest, c);
		} else {
			SDL_Color c = (bone.color == 1) ? SDL_Color{128, 128, 255, 255} : COLOR_WHITE;

			draw_bone_v(bone.x, bone.y, bone.height, c);
		}
	}

	if (bone_stab) {
		if (bone_stab_warn_time > 0.0f) {
			SDL_FRect rect;
			rect.x = border_x1;
			rect.y = border_y2 - bone_stab_dist;
			rect.w = border_x2 - border_x1;
			rect.h = bone_stab_dist;
			SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
			SDL_RenderDrawRectF(game->renderer, &rect);
		} else {
			for (float x = border_x1; x < border_x2; x += 12.0f) {
				float y = border_y2 - bone_stab_dist;
				draw_bone_v(x, y, bone_stab_dist);
			}
		}
	}

	if (clipping) SDL_RenderSetClipRect(game->renderer, nullptr);

	for (GasterBlaster& blaster : blasters) {
		float x = blaster.x;
		float y = blaster.y;
		DrawSprite(&game->spr_gaster_blaster, 0, x, y, blaster.angle, blaster.size, blaster.size);
	}

	DrawText(&game->fnt_determination_mono, attack_list[attack_index], 0, 0);
}

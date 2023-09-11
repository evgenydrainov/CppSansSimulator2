#pragma once

#include "Sprite.h"

#include <vector>
#include <string>

enum struct PlayerMode {
	RED,
	BLUE
};

struct Player {
	float x = 320.0f;
	float y = 304.0f;
	float hsp;
	float vsp;
	PlayerMode mode;
};

struct Bone {
	float x;
	float y;
	int orient;
	float width;
	float height;
	int dir;
	float speed;
	int color;
};

struct GasterBlaster {
	float size = 1.0f;
	float start_x;
	float start_y;
	float end_x;
	float end_y;
	float end_angle;
	float spin_time;
	float blast_time;
	float x;
	float y;
	float angle;
	float timer;
};

struct AttackAction {
	float delay;
	std::string func_name;
	std::vector<std::string> args;
};

using Attack = std::vector<AttackAction>;

struct Battle {
	Player player;
	std::vector<Bone> bones;
	std::vector<GasterBlaster> blasters;

	float border_x1 = 33.0f;
	float border_y1 = 251.0f;
	float border_x2 = 608.0f;
	float border_y2 = 391.0f;
	float border_x1_target = 33.0f;
	float border_y1_target = 251.0f;
	float border_x2_target = 608.0f;
	float border_y2_target = 391.0f;
	float border_speed = 480.0f;
	std::string border_finish_action;

	bool bone_stab;
	int bone_stab_dir;
	float bone_stab_dist;
	float bone_stab_warn_time;
	float bone_stab_stay_time;

	Attack attack;
	size_t attack_action_index;
	float attack_wait_timer;
	bool attack_paused;
	size_t attack_index;
	bool clipping = true;

	void Init();
	void Quit();
	void Update(float delta);
	void Draw(float delta);
};

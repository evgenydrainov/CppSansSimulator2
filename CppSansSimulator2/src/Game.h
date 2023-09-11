#pragma once

#include "Battle.h"

#include "Font.h"

#include <SDL2/SDL_mixer.h>

#include <variant>

#define GAME_W 640
#define GAME_H 480
#define GAME_FPS 60

enum GameState {
	IN_BATTLE,
	OVERWORLD
};

struct Game {
	SDL_Window* window;
	SDL_Renderer* renderer;
	bool quit;
	float time;

	Sprite spr_player_heart;
	Sprite spr_bone_h;
	Sprite spr_bone_v;
	Sprite spr_gaster_blaster;

	SpriteGroup sprite_group;

	Font fnt_determination_mono;

	std::variant<Battle> state;

	void Init();
	void Quit();
	void Run();
	void Frame();
	void Update(float delta);
	void Draw(float delta);
};

extern Game* game;

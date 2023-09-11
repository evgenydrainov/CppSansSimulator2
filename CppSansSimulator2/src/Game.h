#pragma once

#include "Battle.h"

#define GAME_W 640
#define GAME_H 480
#define GAME_FPS 60

enum GameState {
	STATE_IN_BATTLE,
	STATE_OVERWORLD
};

struct Game {
	SDL_Window* window;
	SDL_Renderer* renderer;
	bool quit;
	float time;
	GameState state;

	union {
		Battle battle{};
	};

	void Init();
	void Quit();
	void Run();
	void Frame();
	void Update(float delta);
	void Draw(float delta);
};

extern Game* game;

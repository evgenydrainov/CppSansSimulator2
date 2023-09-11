#pragma once

#include <SDL2/SDL.h>

#define GAME_W 640
#define GAME_H 480
#define GAME_FPS 60

struct Game {
	SDL_Window* window;
	SDL_Renderer* renderer;
	bool quit;
	float time;

	void Init();
	void Quit();
	void Run();
	void Frame();
	void Update(float delta);
	void Draw(float delta);
};

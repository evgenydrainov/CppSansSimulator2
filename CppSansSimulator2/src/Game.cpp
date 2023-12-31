#include "Game.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "misc.h"

#define ASSETS_DIR "assets/"

Game* game = nullptr;

void Game::Init() {
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

	SDL_Init(SDL_INIT_VIDEO
			 | SDL_INIT_AUDIO);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	Mix_Init(0);

	window = SDL_CreateWindow("CppSansSimulator2",
							  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
							  GAME_W, GAME_H,
							  SDL_WINDOW_RESIZABLE);

	renderer = SDL_CreateRenderer(window, -1,
								  SDL_RENDERER_ACCELERATED
								  | SDL_RENDERER_TARGETTEXTURE);

	SDL_RenderSetLogicalSize(renderer, GAME_W, GAME_H);

	InitSpriteGroup(&sprite_group);
	{
		LoadSprite(&spr_player_heart,	&sprite_group, ASSETS_DIR "spr_player_heart.png");
		LoadSprite(&spr_bone_h,			&sprite_group, ASSETS_DIR "spr_bone_h.png");
		LoadSprite(&spr_bone_v,			&sprite_group, ASSETS_DIR "spr_bone_v.png");
		LoadSprite(&spr_gaster_blaster,	&sprite_group, ASSETS_DIR "spr_gaster_blaster.png");
	}
	FinalizeSpriteGroup(&sprite_group);

	LoadFont(&fnt_determination_mono, ASSETS_DIR "DeterminationMono.ttf", 32);

	std::get<0>(state).Init();
}

void Game::Quit() {
	std::visit([](auto&& arg) {
		arg.Quit();
	}, state);

	DestroySpriteGroup(&sprite_group);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void Game::Run() {
	while (!quit) {
		Frame();
	}
}

void Game::Frame() {
	double t = GetTime();

	double frame_end_time = t + (1.0 / (double)GAME_FPS);

	{
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
				case SDL_QUIT: quit = true; break;
			}
		}
	}

	float delta = 1.0f / (float)GAME_FPS;

	Update(delta);

	Draw(delta);

#ifndef __EMSCRIPTEN__
	t = GetTime();
	double time_left = frame_end_time - t;
	if (time_left > 0.0) {
		SDL_Delay((Uint32) (time_left * (0.95 * 1000.0)));
		while (GetTime() < frame_end_time) {}
	}
#endif
}

void Game::Update(float delta) {
	std::visit([delta](auto&& arg) {
		arg.Update(delta);
	}, state);

	time += delta;
}

void Game::Draw(float delta) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	std::visit([delta](auto&& arg) {
		arg.Draw(delta);
	}, state);

	SDL_RenderPresent(renderer);
}

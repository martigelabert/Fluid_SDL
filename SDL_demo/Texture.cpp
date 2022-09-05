#include <SDL.h>
#include <SDL_image.h>
#include "Texture.h"
#include <iostream>


class Texture {

public:
	Texture(const char* path, SDL_Renderer* r) {
		renderer = r;
		surface = IMG_Load(path);
		texture = SDL_CreateTextureFromSurface(r, surface);
		//SDL_FreeSurface(surface);
	}

	int Display() {
		
		if (!SDL_RenderCopy(renderer, texture, NULL, NULL)) return -1;
		
		return 0;
	}

private:
	SDL_Texture* texture;
	SDL_Surface* surface;
	SDL_Renderer* renderer;



};
#include <SDL.h>
#include <iostream>
#include "Texture.cpp"


enum TileType {
	Error, Wall, Air, Random
};

class Tile {};
class Board {};


int main(int argc, char *argv[]) 
{
	const char* title = "SDL Window";
	int wight = 800;
	int height = 600;

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* w = SDL_CreateWindow(title,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wight,height,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	
	
	SDL_Renderer* r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);

	if (w && r) {
		bool done = false;
		SDL_RenderPresent(r);

		Texture t{ "assets/a.JPEG", r };

		SDL_Event e;
		while (!done) {

			//SDL_SetRenderDrawColor(r, 0, 0, 0, SDL_ALPHA_OPAQUE);
			SDL_RenderClear(r);

			t.Display();
			
			//SDL_SetRenderDrawColor(r, 255, 255, 255, SDL_ALPHA_OPAQUE);
			/*SDL_RenderDrawLine(r, 320, 200, 300, 240);
			SDL_RenderDrawLine(r, 300, 240, 340, 240);
			SDL_RenderDrawLine(r, 340, 240, 320, 200);*/
			SDL_RenderPresent(r);

			while (SDL_PollEvent(&e)) {
				switch (e.type)
				{
				case SDL_QUIT: done = true; break;
				default: break;
				}
			}	
		}

		// Al terminar de ejecutar eliminaremos tanto el render como
		// la ventana
		if (r) SDL_DestroyRenderer(r);
		if (r) SDL_DestroyWindow(w);
	}

	// Eliminaremos la sesion de SDL y terminaremos la ejecución
	SDL_Quit();

	return 0;
}
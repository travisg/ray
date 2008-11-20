// ray.cpp : Defines the entry point for the console application.
//

#include <cstdio>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

SDL_Surface *gScreen;

int SetupSDL()
{
	atexit(SDL_Quit);

	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
	SDL_EnableUNICODE(1);

	gScreen = SDL_SetVideoMode(1024, 768, 32, SDL_SWSURFACE);
	SDL_UpdateRect(gScreen, 0,0,0,0); // Update entire surface

	SDL_WM_SetCaption("Ray","ray");

	return 0;
}

int main(int argc, char* argv[])
{
	printf("hello\n");

	SetupSDL();

	SDL_Event event;
	bool quit = false;

	while(!quit) {
		SDL_WaitEvent(&event);

		switch (event.type) {
			case SDL_KEYDOWN:
				printf("The %s (0x%x) key was pressed!\n", SDL_GetKeyName(event.key.keysym.sym), event.key.keysym.unicode);
				break;
			case SDL_QUIT:
				quit = true;
				break;
		}
	}

	return 0;
}


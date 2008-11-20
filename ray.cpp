// ray.cpp : Defines the entry point for the console application.
//

#include <cstdio>

#include <sdl.h>
#include <RenderSurface.h>

RenderSurface *gRenderSurface;
SDL_Surface *gScreen;

static void ScreenNotify(int x, int y, color32 color)
{
	printf("notify %d %d %u\n", x, y, color);

	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.h = 1;
	rect.w = 1;
	SDL_FillRect(gScreen, &rect, color);

	SDL_UpdateRect(gScreen, x,y,1,1);
}

int SetupSDL()
{
	atexit(SDL_Quit);

	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
	SDL_EnableUNICODE(1);

	gScreen = SDL_SetVideoMode(gRenderSurface->Width(), gRenderSurface->Height(), 32, SDL_SWSURFACE);
	SDL_UpdateRect(gScreen, 0,0,0,0); // Update entire surface

	SDL_WM_SetCaption("Ray","ray");

	gRenderSurface->SetNotification(ScreenNotify);

	return 0;
}

int main(int argc, char* argv[])
{
	printf("hello\n");

	gRenderSurface = new RenderSurface(1024, 768);
	SetupSDL();

	SDL_Event event;
	bool quit = false;

	for (int i  = 0; i < 100000; i++) {
		gRenderSurface->SetXY(rand() % 1024, rand() % 768, rand());
	}

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


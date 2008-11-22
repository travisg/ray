// ray.cpp : Defines the entry point for the console application.
//

#include <cstdio>

#include <sdl.h>
#include <Ray.h>
#include <RenderSurface.h>
#include <Tracer.h>

RenderSurface *gRenderSurface;
SDL_Surface *gScreen;

// track dirty screen
volatile bool dirty = false;

static void ScreenNotify(int x, int y, color32 color)
{
//	printf("notify %d %d %u\n", x, y, color);

	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.h = 1;
	rect.w = 1;
	SDL_FillRect(gScreen, &rect, color);

	dirty = true;
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

int TracerThread(void *data)
{
	std::cout << "TracerThread start" << std::endl;
	
	Tracer *tracer = new Tracer(*gRenderSurface);
	tracer->Trace();

	gRenderSurface->WriteTGAFile("foo.tga");

	delete tracer;

	std::cout << "TracerThread end" << std::endl;

	return 0;
}

Uint32 TimerTick(Uint32 interval, void *param)
{
	if (dirty) {
		SDL_UpdateRect(gScreen, 0, 0, 0, 0);
		dirty = false;
	}

	return interval;
}

int main(int argc, char* argv[])
{
	printf("hello\n");

	srand(time(NULL));

	gRenderSurface = new RenderSurface(800, 600);
	SetupSDL();

	SDL_Thread *thread = SDL_CreateThread(&TracerThread, gRenderSurface);

	SDL_AddTimer(100, &TimerTick, NULL);

	// main sdl thread loop
	SDL_Event event;
	bool quit = false;

	while(!quit) {
		SDL_WaitEvent(&event);

		switch (event.type) {
#if 0
			case SDL_KEYDOWN:
				printf("The %s (0x%x) key was pressed!\n", SDL_GetKeyName(event.key.keysym.sym), event.key.keysym.unicode);
				break;
#endif
			case SDL_QUIT:
				quit = true;
				break;
		}
	}

	delete gRenderSurface;

	return 0;
}


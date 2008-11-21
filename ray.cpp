// ray.cpp : Defines the entry point for the console application.
//

#include <cstdio>
#include <values.h>

#include <sdl.h>
#include <Ray.h>
#include <RenderSurface.h>
#include <Tracer.h>

RenderSurface *gRenderSurface;
SDL_Surface *gScreen;

bool dirty = false;
int dirty_left_x = MAXINT;
int dirty_left_y = MAXINT;
int dirty_right_x = -1;
int dirty_right_y = -1;

static void ScreenNotify(int x, int y, color32 color)
{
//	printf("notify %d %d %u\n", x, y, color);

	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.h = 1;
	rect.w = 1;
	SDL_FillRect(gScreen, &rect, color);

//	SDL_UpdateRect(gScreen, x,y,1,1);
	if (x < dirty_left_x)
		dirty_left_x = x;
	if (y < dirty_left_y)
		dirty_left_y = y;	
	if (x > dirty_right_x)
		dirty_right_x = x;
	if (y > dirty_right_y)
		dirty_right_y = y;
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

int main(int argc, char* argv[])
{
	printf("hello\n");

	srand(time(NULL));

	gRenderSurface = new RenderSurface(800, 800);
	SetupSDL();

	Tracer *tracer = new Tracer(*gRenderSurface);

	SDL_Event event;
	bool quit = false;

	tracer->Trace();

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

	delete tracer;
	delete gRenderSurface;

	return 0;
}


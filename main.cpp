// ray.cpp : Defines the entry point for the console application.
//

#include <cstdio>
#include <iostream>
#include <time.h>

#include <sdl.h>
#include <Ray.h>
#include <RenderSurface.h>
#include <Tracer.h>
#include <Scene.h>
#include <DisplayWindow.h>

RenderSurface *gRenderSurface;
DisplayWindow *gWindow;

int SetupSDL()
{
	atexit(SDL_Quit);

	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
	SDL_EnableUNICODE(1);

	return 0;
}

int TracerThread(void *data)
{
	std::cout << "TracerThread start" << std::endl;
	time_t start = time(NULL);

	Scene *scene = new Scene();
	
	Tracer *tracer = new Tracer(*gRenderSurface, *scene);
	tracer->Trace();

	start = time(NULL) - start;
	std::cout << "trace end, took " << start << " seconds" << std::endl;

	gRenderSurface->WriteTGAFile("foo.tga");

	delete tracer;
	delete scene;

	std::cout << "TracerThread end" << std::endl;

	return 0;
}

int main(int argc, char* argv[])
{
	printf("hello\n");

	srand(time(NULL));

	SetupSDL();

	gRenderSurface = new RenderSurface(1600, 1200);

	gWindow = new DisplayWindow(gRenderSurface->Width(), gRenderSurface->Height());
	gWindow->SetRenderSurface(*gRenderSurface);
	gWindow->CreateWindow();

	SDL_CreateThread(&TracerThread, gRenderSurface);

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


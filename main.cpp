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
Scene *gScene;
TraceMaster *gTraceMaster;

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

	Tracer *tracer = new Tracer(*gRenderSurface, *gScene, *gTraceMaster);
	tracer->Trace();

	start = time(NULL) - start;
	std::cout << "trace end, took " << start << " seconds" << std::endl;

	delete tracer;

	std::cout << "TracerThread end" << std::endl;

	return 0;
}

int main(int argc, char* argv[])
{
	printf("hello\n");

	srand(time(NULL));

	SetupSDL();

	gRenderSurface = new RenderSurface(800, 600);

	gWindow = new DisplayWindow(800, 600);
	gWindow->SetRenderSurface(*gRenderSurface);
	gWindow->CreateWindow();

	/* create the scene */
	gScene = new Scene();

	/* create a tracemaster */
	gTraceMaster = new TraceMasterSimple(*gRenderSurface);
//	gTraceMaster = new TraceMasterRandom(*gRenderSurface);

	for (int i = 0; i < 4; i++)
		SDL_CreateThread(&TracerThread, NULL);

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
				std::cout << "SDL_QUIT\n";
				quit = true;
				break;
		}
	}

	/* super cheezy mechanism to try to kill all the tracer threads */
	gTraceMaster->Halt();
	sleep(1);

	std::cout << "writing output file..." << std::flush;
	gRenderSurface->WriteTGAFile("foo.tga");
	std::cout << "done" << std::endl;

	delete gRenderSurface;
	delete gTraceMaster;
	delete gScene;

	return 0;
}


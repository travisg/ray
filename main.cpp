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

	SDL_Init(SDL_INIT_TIMER);
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

//	gRenderSurface = new RenderSurface(3200 * 2, 1600 * 2);
//	gRenderSurface = new RenderSurface(3200, 1600);
	gRenderSurface = new RenderSurface(800, 600);

	gRenderSurface->OpenOutFile("out.ray");

//	gWindow = new DisplayWindow(800, 600);
//	gWindow->SetRenderSurface(*gRenderSurface);
//	gWindow->CreateWindow();

	/* create the scene */
	gScene = new Scene();

	/* create a tracemaster */
	gTraceMaster = new TraceMasterSimple(*gRenderSurface);
//	gTraceMaster = new TraceMasterRandom(*gRenderSurface);

	for (int i = 0; i < 16; i++)
		SDL_CreateThread(&TracerThread, NULL);

	gTraceMaster->WaitForDone();

//	std::cout << "writing output file..." << std::flush;
//	gRenderSurface->WriteTGAFile("foo.tga");
//	std::cout << "done" << std::endl;

	delete gRenderSurface;
	delete gTraceMaster;
	delete gScene;

	return 0;
}


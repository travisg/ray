#include <cstdio>
#include <iostream>
#include <time.h>

#include <Ray.h>
#include <RenderSurface.h>
#include <Tracer.h>
#include <Scene.h>

RenderSurface *gRenderSurface;
Scene *gScene;
TraceMaster *gTraceMaster;

#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/thread.hpp>

namespace po = boost::program_options;

int TracerThread()
{
//	std::cout << "TracerThread start" << std::endl;
	time_t start = time(NULL);

	Tracer *tracer = new Tracer(*gRenderSurface, *gScene, *gTraceMaster);
	tracer->Trace();

	start = time(NULL) - start;
	std::cout << "trace end, took " << start << " seconds" << std::endl;

	delete tracer;

//	std::cout << "TracerThread end" << std::endl;

	return 0;
}

int main(int argc, char* argv[])
{
	srand(time(NULL));

	int xres, yres, cpus;
	std::string outfile;

	// deal with options
	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "help message")
		("xres", po::value<int>(&xres)->default_value(800), "x resolution")
		("yres", po::value<int>(&yres)->default_value(600), "y resolution")
		("cpus,c", po::value<int>(&cpus)->default_value(8), "number cpus")
		("out,o", po::value<std::string>(&outfile)->default_value(std::string("out.ray")), "output file")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 1;
	}

	printf("xres %d yres %d cpus %d\n", xres, yres, cpus);
	std::cout << "outfile " << outfile << std::endl;

	gRenderSurface = new RenderSurfaceFile(xres, yres);

	(dynamic_cast<RenderSurfaceFile *>(gRenderSurface))->OpenOutFile(outfile);

	/* create the scene */
	gScene = new Scene();

	/* create a tracemaster */
	gTraceMaster = new TraceMasterSimple(*gRenderSurface);
//	gTraceMaster = new TraceMasterRandom(*gRenderSurface);

	boost::thread **threads = new boost::thread *[cpus];
	for (int i = 0; i < cpus; i++) {
		threads[i] = new boost::thread(&TracerThread);
	}

	// wait for the trace master to quit handing out work units
	gTraceMaster->WaitForDone();

	// wait for the worker threads to complete
	std::cout << "waiting for worker threads to shut down" << std::endl;
	for (int i = 0; i < cpus; i++) {
		threads[i]->join();
		delete threads[i];
	}

//	std::cout << "writing output file..." << std::flush;
//	gRenderSurface->WriteTGAFile("foo.tga");
//	std::cout << "done" << std::endl;

	std::cout << "shutting down" << std::endl;

	delete gRenderSurface;
	delete gTraceMaster;
	delete gScene;

	return 0;
}


#include <iostream>
#include <inttypes.h>
#include "RayFile.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

static void usage(int argc, char **argv)
{
	printf("usage: %s <input file> <x> <y>\n", argv[0]);
}

int main(int argc, char **argv)
{
	if (argc < 4) {
		usage(argc, argv);
		exit(1);
	}

	RayFile file;

	std::string inname(argv[1]);
	printf("infile = %s\n", inname.c_str());

	size_t outpos = inname.find_last_of("/\\");
	std::string outname = inname.substr(outpos + 1);
	printf("outfile = %s\n", outname.c_str());

	int rc = file.Open(argv[1]);
	if (rc < 0) {
		std::cerr << "error opening file " << argv[1] << std::endl;
		return 1;
	}

	int sizex = atoi(argv[2]);
	int sizey = atoi(argv[3]);

	if (sizex <= 0 || sizey <= 0) {	
		std::cerr << " invalid X or Y argument" << std::endl;
		return 1;
	}

	float *buf;
	uint64_t len;
	rc = ReadIntoMmap(file, outname + ".float", &buf, &len);
	if (rc < 0) {
		std::cerr << "failed to mmap file" << std::endl;
		return 1;
	}
	
	printf("sizex %d sizey %d\n", sizex, sizey);
	for (int y = 0; y < file.Height(); y += sizey) {
		// see how much we can actually do in this chunk
		int runy = sizey;
		if (y + runy > file.Height())
			runy = file.Height() - y;
		for (int x = 0; x < file.Width(); x += sizex) {
			RayFile outfile;

			// how much X can we actually do?
			int runx = sizex;
			if (x + runx > file.Width())
				runx = file.Width() - x;

			char name[512];
			snprintf(name, sizeof(name), "%s-%d.%d", outname.c_str(), x, y);

			printf("outputting %s: size %dx%d, x %d y %d\n", name, runx, runy, x, y);

			outfile.OpenWrite(name, runx, runy);

			for (int y1 = y; y1 < y + runy; y1++) {
//				printf("setxyrun %d %d %d\n", x, y1, runx);
				outfile.SetXYRun(0, y1 - y, runx, &buf[(y1 * file.Width() + x) * 3]);
			}
			outfile.Close();
		}
	}

	/* close it all down */
	munmap(buf, len);

	return 0;
}

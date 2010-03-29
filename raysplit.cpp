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
	
	// read in the input file into a memory mapped file
	int fd = open((argv[1] + std::string(".float")).c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
	int64_t len = file.GetPixelCount() * 3;

	rc = ftruncate(fd, len * sizeof(float));
	if (rc < 0) {
		std::cout << rc << " " << errno << " " << strerror(errno) << std::endl;
		return 1;
	}

	float *buf = (float *)mmap(NULL, len * sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	std::cout << "converting " << argv[1] << " to pure float array" << std::endl;
	if (file.ReadIntoBuffer(buf) < 0) {
		return -1;
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
			snprintf(name, sizeof(name), "%s-%d.%d", argv[1], x, y);

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
	munmap(buf, len * sizeof(float));
	close(fd);
	file.Close();

	return 0;
}

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
	printf("usage: %s <input file> <output file>\n", argv[0]);
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		usage(argc, argv);
		exit(1);
	}

	RayFile file;

	int rc = file.Open(argv[1]);
	if (rc < 0) {
		std::cerr << "error opening file " << argv[1] << std::endl;
		return 1;
	}

	rc = file.ConvertToTGA(argv[2]);
	if (rc < 0) {
		std::cerr << "error outputting file " << argv[2] << std::endl;
		return 1;
	}

	file.Close();

	return 0;
}

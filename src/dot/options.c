#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dot/common.h>


static void print_options_help(const char* program_name) {
	printf(
"%s -s array-size [-i iterations]\n"
"Required parameters:\n"
"  -s   --array-size       The size of array, in bytes, processed in micro-kernel (usually half or level-n cache size)\n"
"Optional parameters:\n"
"  -i   --iterations       The number of benchmark iterations (default: 1000)\n",
		program_name);
}

struct benchmark_options parse_options(int argc, char** argv) {
	struct benchmark_options options = {
		.iterations = 1000,
		.array_size = 0,
	};
	for (int argi = 1; argi < argc; argi += 1) {
		if ((strcmp(argv[argi], "--array-size") == 0) || (strcmp(argv[argi], "-s") == 0)) {
			if (argi + 1 == argc) {
				fprintf(stderr, "Error: expected array size value\n");
				exit(EXIT_FAILURE);
			}
			if (sscanf(argv[argi + 1], "%zu", &options.array_size) != 1) {
				fprintf(stderr, "Error: can not parse %s as an unsigned integer\n", argv[argi + 1]);
				exit(EXIT_FAILURE);
			}
			if (options.array_size == 0) {
				fprintf(stderr, "Error: invalid value %s for the array size: positive value expected\n", argv[argi + 1]);
				exit(EXIT_FAILURE);
			}
			argi += 1;
		} else if ((strcmp(argv[argi], "--iterations") == 0) || (strcmp(argv[argi], "-i") == 0)) {
			if (argi + 1 == argc) {
				fprintf(stderr, "Error: expected iterations value\n");
				exit(EXIT_FAILURE);
			}
			if (sscanf(argv[argi + 1], "%zu", &options.iterations) != 1) {
				fprintf(stderr, "Error: can not parse %s as an unsigned integer\n", argv[argi + 1]);
				exit(EXIT_FAILURE);
			}
			if (options.iterations == 0) {
				fprintf(stderr, "Error: invalid value %s for the number of iterations: positive value expected\n", argv[argi + 1]);
				exit(EXIT_FAILURE);
			}
			argi += 1;
		} else if ((strcmp(argv[argi], "--help") == 0) || (strcmp(argv[argi], "-h") == 0)) {
			print_options_help(argv[0]);
			exit(EXIT_SUCCESS);
		} else {
			fprintf(stderr, "Error: unknown argument '%s'\n", argv[argi]);
			print_options_help(argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	if (options.array_size == 0) {
		fprintf(stderr, "Error: the block size is not specified\n");
		print_options_help(argv[0]);
		exit(EXIT_FAILURE);
	}
	return options;
}

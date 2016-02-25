#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <low-level/common.h>


static void print_options_help(const char* program_name) {
	printf(
"%s -t type [-i iterations] [-r repeats]\n"
"Required parameters:\n"
"  -t   --type         The type of benchmark:\n"
"                          doubledouble-latency\n"
"                          doubledouble-throughput\n"
#ifdef FPPLUS_HAVE_FLOAT128
"                          quad-latency\n"
#endif
"                          polevl-latency\n"
"Optional parameters:\n"
"  -i   --iterations   The number of benchmark iterations (default: 1000)\n"
"  -r   --repeats      The number of repeats within the benchmark iteration (default: 1024)\n",
		program_name);
}

struct benchmark_options parse_options(int argc, char** argv) {
	struct benchmark_options options = {
		.type = benchmark_type_none,
		.iterations = 1000,
		.repeats = 1024,
	};
	for (int argi = 1; argi < argc; argi += 1) {
		if ((strcmp(argv[argi], "--type") == 0) || (strcmp(argv[argi], "-t") == 0)) {
			if (argi + 1 == argc) {
				fprintf(stderr, "Error: expected benchmark type\n");
				exit(EXIT_FAILURE);
			}
			if (strcmp(argv[argi + 1], "doubledouble-latency") == 0) {
				options.type = benchmark_type_doubledouble_latency;
			} else if (strcmp(argv[argi + 1], "doubledouble-throughput") == 0) {
				options.type = benchmark_type_doubledouble_throughput;
#ifdef FPPLUS_HAVE_FLOAT128
			} else if (strcmp(argv[argi + 1], "quad-latency") == 0) {
				options.type = benchmark_type_quad_latency;
#endif
			} else if (strcmp(argv[argi + 1], "polevl-latency") == 0) {
				options.type = benchmark_type_polevl_latency;
			} else {
				fprintf(stderr, "Error: invalid benchmark type %s\n", argv[argi + 1]);
				exit(EXIT_FAILURE);
			}
			argi += 1;
		} else if ((strcmp(argv[argi], "--repeats") == 0) || (strcmp(argv[argi], "-r") == 0)) {
			if (argi + 1 == argc) {
				fprintf(stderr, "Error: expected repeats value\n");
				exit(EXIT_FAILURE);
			}
			if (sscanf(argv[argi + 1], "%zu", &options.repeats) != 1) {
				fprintf(stderr, "Error: can not parse %s as an unsigned integer\n", argv[argi + 1]);
				exit(EXIT_FAILURE);
			}
			if (options.repeats == 0) {
				fprintf(stderr, "Error: invalid value %s for the array repeats: positive value expected\n", argv[argi + 1]);
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
	if (options.type == 0) {
		fprintf(stderr, "Error: the benchmark type is not specified\n");
		print_options_help(argv[0]);
		exit(EXIT_FAILURE);
	}
	return options;
}

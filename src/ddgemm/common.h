#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <ddgemm/ddgemm.h>


struct benchmark_options {
	size_t iterations;
	size_t block_size;
};

struct benchmark_options parse_options(int argc, char** argv);

#ifdef __cplusplus
} /* extern "C" */
#endif

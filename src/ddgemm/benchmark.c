#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>

#include <utils.h>
#include <ddgemm/common.h>


static void benchmark(
	ddgemm_function ddgemm,
	size_t nr, size_t mr,
	size_t iterations,
	size_t block_size,
	const double a[restrict],
	const doubledouble b[restrict],
	doubledouble c[restrict])
{
	double iteration_times[iterations];
	const size_t kc = block_size / ((nr + mr) * sizeof(doubledouble));
	for (size_t iteration = 0; iteration < iterations; iteration++) {
		const double start_time = high_precision_time();

		ddgemm(kc, a, b, c);

		iteration_times[iteration] = high_precision_time() - start_time;
	}
	const double median_time_ns = median_double(iteration_times, iterations);
	const double gflops = 2.0 * nr * mr * kc / median_time_ns;
	printf("%zu\t" "%zu\t" "%zu\t" "%zu\t" "%.1lf\n", block_size, mr, nr, kc, gflops * 1000.0);
}

int main(int argc, char *argv[]) {
    const struct benchmark_options options = parse_options(argc, argv);

	void* a_array = valloc(options.block_size);
	void* b_array = valloc(options.block_size);
	void* c_array = valloc(DDGEMM_MR_MAX * DDGEMM_NR_MAX * sizeof(doubledouble));
	for (double* double_array = a_array; double_array != a_array + options.block_size; double_array++) {
		*double_array = M_PI;
	}
	for (double* double_array = b_array; double_array != b_array + options.block_size; double_array++) {
		*double_array = M_E;
	}
	memset(c_array, 0, DDGEMM_MR_MAX * DDGEMM_NR_MAX * sizeof(doubledouble));

	for (size_t mr = DDGEMM_MR_MIN; mr <= DDGEMM_MR_MAX; mr += DDGEMM_MR_STEP) {
		for (size_t nr = DDGEMM_NR_MIN; nr <= DDGEMM_NR_MAX; nr += 1) {
			ddgemm_function ddgemm = select_ddgemm_kernel(mr, nr);
			benchmark(ddgemm, mr, nr, options.iterations, options.block_size, a_array, b_array, c_array);
		}
	}

	free(a_array);
	free(b_array);
	free(c_array);
}

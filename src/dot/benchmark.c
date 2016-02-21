#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>

#include <utils.h>
#include <dot/common.h>


static void benchmark_dot_product(
    dot_product_function dot,
    const char* name,
    size_t unroll_factor,
    size_t iterations,
    size_t elements, const double a[restrict static elements], const double b[restrict static elements])
{
    uint64_t iteration_ticks[iterations];
    for (size_t iteration = 0; iteration < iterations; iteration++) {
        const uint64_t start_ticks = cpu_ticks();

        dot(elements, a, b);

        iteration_ticks[iteration] = cpu_ticks() - start_ticks;
    }
    const uint64_t median_ticks = median_uint64(iteration_ticks, iterations);
    printf("double\t" "%s\t" "%zu\t" "%10zu\t" "%.2lf\n",
        name, unroll_factor, elements, ((double) median_ticks) / ((double) elements));
}

static void benchmark_compensated_dot_product(
    compensated_dot_product_function dot,
    const char* name,
    size_t unroll_factor,
    size_t iterations,
    size_t elements, const double a[restrict static elements], const double b[restrict static elements])
{
    uint64_t iteration_ticks[iterations];
    for (size_t iteration = 0; iteration < iterations; iteration++) {
        const uint64_t start_ticks = cpu_ticks();

        dot(elements, a, b);

        iteration_ticks[iteration] = cpu_ticks() - start_ticks;
    }
    const uint64_t median_ticks = median_uint64(iteration_ticks, iterations);
    printf("compensated\t" "%s\t" "%zu\t" "%10zu\t" "%.2lf\n",
        name, unroll_factor, elements, ((double) median_ticks) / ((double) elements));
}

int main(int argc, char *argv[]) {
    const struct benchmark_options options = parse_options(argc, argv);

    void* a_array = valloc(options.array_size);
    void* b_array = valloc(options.array_size);
    for (double* double_array = a_array; double_array != a_array + options.array_size; double_array++) {
        *double_array = M_PI;
    }
    for (double* double_array = b_array; double_array != b_array + options.array_size; double_array++) {
        *double_array = M_E;
    }
    const size_t array_elements = options.array_size / sizeof(double);

    benchmark_dot_product(dot_product_muladd_unroll1, "mul+add", 1,
        options.iterations, array_elements, a_array, b_array);
    benchmark_dot_product(dot_product_muladd_unroll2, "mul+add", 2,
        options.iterations, array_elements, a_array, b_array);
    benchmark_dot_product(dot_product_muladd_unroll3, "mul+add", 3,
        options.iterations, array_elements, a_array, b_array);
    benchmark_dot_product(dot_product_muladd_unroll4, "mul+add", 4,
        options.iterations, array_elements, a_array, b_array);
    benchmark_dot_product(dot_product_muladd_unroll5, "mul+add", 5,
        options.iterations, array_elements, a_array, b_array);
    benchmark_dot_product(dot_product_muladd_unroll6, "mul+add", 6,
        options.iterations, array_elements, a_array, b_array);
    benchmark_dot_product(dot_product_muladd_unroll7, "mul+add", 7,
        options.iterations, array_elements, a_array, b_array);
    benchmark_dot_product(dot_product_muladd_unroll8, "mul+add", 8,
        options.iterations, array_elements, a_array, b_array);

    benchmark_dot_product(dot_product_fma_unroll1, "fma", 1,
        options.iterations, array_elements, a_array, b_array);
    benchmark_dot_product(dot_product_fma_unroll2, "fma", 2,
        options.iterations, array_elements, a_array, b_array);
    benchmark_dot_product(dot_product_fma_unroll3, "fma", 3,
        options.iterations, array_elements, a_array, b_array);
    benchmark_dot_product(dot_product_fma_unroll4, "fma", 4,
        options.iterations, array_elements, a_array, b_array);
    benchmark_dot_product(dot_product_fma_unroll5, "fma", 5,
        options.iterations, array_elements, a_array, b_array);
    benchmark_dot_product(dot_product_fma_unroll6, "fma", 6,
        options.iterations, array_elements, a_array, b_array);
    benchmark_dot_product(dot_product_fma_unroll7, "fma", 7,
        options.iterations, array_elements, a_array, b_array);
    benchmark_dot_product(dot_product_fma_unroll8, "fma", 8,
        options.iterations, array_elements, a_array, b_array);

    benchmark_compensated_dot_product(compensated_dot_product_efmuladd_unroll1, "efmul+efadd", 1,
        options.iterations, array_elements, a_array, b_array);
    benchmark_compensated_dot_product(compensated_dot_product_efmuladd_unroll2, "efmul+efadd", 2,
        options.iterations, array_elements, a_array, b_array);
    benchmark_compensated_dot_product(compensated_dot_product_efmuladd_unroll3, "efmul+efadd", 3,
        options.iterations, array_elements, a_array, b_array);
    benchmark_compensated_dot_product(compensated_dot_product_efmuladd_unroll4, "efmul+efadd", 4,
        options.iterations, array_elements, a_array, b_array);
    benchmark_compensated_dot_product(compensated_dot_product_efmuladd_unroll5, "efmul+efadd", 5,
        options.iterations, array_elements, a_array, b_array);
    benchmark_compensated_dot_product(compensated_dot_product_efmuladd_unroll6, "efmul+efadd", 6,
        options.iterations, array_elements, a_array, b_array);
    benchmark_compensated_dot_product(compensated_dot_product_efmuladd_unroll7, "efmul+efadd", 7,
        options.iterations, array_elements, a_array, b_array);
    benchmark_compensated_dot_product(compensated_dot_product_efmuladd_unroll8, "efmul+efadd", 8,
        options.iterations, array_elements, a_array, b_array);

    free(a_array);
    free(b_array);
}

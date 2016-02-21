#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>

#include <utils.h>
#include <low-level/common.h>

#ifndef __KNC__
    #define TUPLE doubledouble
    #define ELEMENTS_PER_TUPLE 1
#else
    #define TUPLE __m512dd
    #define ELEMENTS_PER_TUPLE (sizeof(__m512dd) / sizeof(doubledouble))
#endif

static void warmup(size_t elements, const doubledouble array[restrict static elements / ELEMENTS_PER_TUPLE]) {
    double start_ns = high_precision_time();
    const size_t warmup_iterations = 1024;
    for (size_t iteration = 0; iteration < warmup_iterations; iteration++) {
        vsum(elements / ELEMENTS_PER_TUPLE, (const TUPLE*) array);
        /* Stop if warmup for over 1 second */
        if (high_precision_time() - start_ns >= 1.0e+9)
            break;
    }
}

#ifdef HAVE_FLOAT128
static void benchmark_quad(
    benchmark_quad_function function, const char* operation_name,
    size_t iterations, size_t elements, __float128 array[restrict static elements])
{
    uint64_t min_ticks = UINT64_MAX;
    for (size_t iteration = 0; iteration < iterations; iteration++) {
        const uint64_t start_ticks = cpu_ticks();
        function(elements, array);
        const uint64_t total_ticks = cpu_ticks() - start_ticks;
        if (total_ticks < min_ticks)
            min_ticks = total_ticks;
    }
    printf("%s\t" "%10zu\t" "%.2lf\n", operation_name, elements, ((double) min_ticks) / ((double) elements));
}
#endif

static void benchmark_doubledouble(
    benchmark_doubledouble_function function, const char* operation_name,
    size_t iterations, size_t elements,
    TUPLE array[restrict static elements / ELEMENTS_PER_TUPLE])
{
    uint64_t min_ticks = UINT64_MAX;
    for (size_t iteration = 0; iteration < iterations; iteration++) {
        const uint64_t start_ticks = cpu_ticks();
        function(elements / ELEMENTS_PER_TUPLE, (TUPLE*) array);
        const uint64_t total_ticks = cpu_ticks() - start_ticks;
        if (total_ticks < min_ticks)
            min_ticks = total_ticks;
    }
    printf("%s\t" "%10zu\t" "%.2lf\n", operation_name, elements, ((double) min_ticks) / ((double) (elements / ELEMENTS_PER_TUPLE)));
}

static void benchmark_polevl(
    benchmark_polevl_function function, const char* operation_name, size_t iterations, size_t repeats)
{
    uint64_t min_ticks = UINT64_MAX;
    for (size_t iteration = 0; iteration < iterations; iteration++) {
        const uint64_t start_ticks = cpu_ticks();
        #ifndef __KNC__
            function(M_PI, repeats);
        #else
            function(_mm512_set1_pd(M_PI), repeats);
        #endif
        const uint64_t total_ticks = cpu_ticks() - start_ticks;
        if (total_ticks < min_ticks)
            min_ticks = total_ticks;
    }
    printf("%s\t" "%10zu\t" "%.2lf\n", operation_name, repeats, ((double) min_ticks) / ((double) repeats));
}

int main(int argc, char *argv[]) {
    const struct benchmark_options options = parse_options(argc, argv);

    const size_t array_size = options.repeats * 16;
    void* v_array = NULL;
    switch (options.type) {
        case benchmark_type_doubledouble_latency:
        case benchmark_type_doubledouble_throughput:
#ifdef HAVE_FLOAT128
        case benchmark_type_quad_latency:
#endif
            v_array = valloc(array_size);
            for (doubledouble* dd_array = v_array; dd_array != v_array + array_size; dd_array++) {
                *dd_array = (doubledouble) { 1.0, 0.0 };
            }
            warmup(options.repeats, v_array);
            break;
        case benchmark_type_polevl_latency:
            break;
        case benchmark_type_none:
            __builtin_unreachable();
    }

    switch (options.type) {
        case benchmark_type_doubledouble_latency:
            benchmark_doubledouble((benchmark_doubledouble_function) vsum,
                "DDADD\tLatency", options.iterations, options.repeats, v_array);
            benchmark_doubledouble((benchmark_doubledouble_function) vprod,
                "DDMUL\tLatency", options.iterations, options.repeats, v_array);
            break;
        case benchmark_type_doubledouble_throughput:
            benchmark_doubledouble(vaddc_helper,
                "DDADD\tThroughput", options.iterations, options.repeats, v_array);
            benchmark_doubledouble(vmulc_helper,
                "DDMUL\tThroughput", options.iterations, options.repeats, v_array);
            break;
#ifdef HAVE_FLOAT128
        case benchmark_type_quad_latency:
            for (__float128* q_array = v_array; q_array != v_array + array_size; q_array++) {
                *q_array = 1.235412347891354098213343278Q;
            }
            benchmark_quad(qsum,
                "QADD\tLatency", options.iterations, options.repeats, v_array);
            benchmark_quad(qprod,
                "QMUL\tLatency", options.iterations, options.repeats, v_array);
            break;
#endif
        case benchmark_type_polevl_latency:
            benchmark_polevl(benchmark_compensated_horner15, "HORNER/COMP\tLatency", options.iterations, options.repeats);
            benchmark_polevl(benchmark_fma_horner15, "HORNER/FMA\tLatency", options.iterations, options.repeats);
            benchmark_polevl(benchmark_muladd_horner15, "HORNER/MAC\tLatency", options.iterations, options.repeats);
            break;
        case benchmark_type_none:
            __builtin_unreachable();
    }
    free(v_array);
}

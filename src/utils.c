#include <stdlib.h>

#include <utils.h>

static int compare_double(const void *a_ptr, const void *b_ptr) {
    const double a = *((const double*) a_ptr);
    const double b = *((const double*) b_ptr);
    return (a > b) - (a < b);
}

static int compare_uint64(const void *a_ptr, const void *b_ptr) {
    const uint64_t a = *((const uint64_t*) a_ptr);
    const uint64_t b = *((const uint64_t*) b_ptr);
    return (a > b) - (a < b);
}

double median_double(double array[], size_t length) {
    qsort(array, length, sizeof(double), &compare_double);
    if (length % 2 == 0) {
        const double median_lo = array[length / 2 - 1];
        const double median_hi = array[length / 2];
        return 0.5 * (median_lo + median_hi);
    } else {
        return array[length / 2];
    }
}

uint64_t median_uint64(uint64_t array[], size_t length) {
    qsort(array, length, sizeof(uint64_t), &compare_uint64);
    if (length % 2 == 0) {
        const uint64_t median_lo = array[length / 2 - 1];
        const uint64_t median_hi = array[length / 2];
        return (median_lo + median_hi + 1) / 2;
    } else {
        return array[length / 2];
    }
}

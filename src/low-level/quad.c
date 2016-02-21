#include <low-level/common.h>

/*
 * Benchmarks of quad-precision operations
 */


/* Chained sum of array elements - benchmark for addition latency */
__float128 qsum(size_t array_elements, const __float128 array[restrict static array_elements]) {
	__float128 sum = 0.0Q;
	do {
		sum += (*array++);
	} while (--array_elements);
	return sum;
}

/* Chained product of array elements - benchmark for multiplication latency */
__float128 qprod(size_t array_elements, const __float128 array[restrict static array_elements]) {
	__float128 prod = 1.0Q;
	do {
		prod *= (*array++);
	} while (--array_elements);
	return prod;
}

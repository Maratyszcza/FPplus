#include <low-level/common.h>

#include <fpplus.h>

/*
 * Benchmarks of double-double precision operations
 * Note: on Xeon Phi scalar operations require setting up mask register, which may affect performance,
 * so we have special versions for Xeon Phi which operate on whole SIMD vectors.
 */

/* Chained sum of array elements - benchmark for addition latency */
#ifndef __KNC__
	doubledouble vsum(size_t array_elements, const doubledouble array[restrict static array_elements]) {
		doubledouble sum = { 0.0, 0.0 };
		do {
			sum = ddadd(sum, *array++);
		} while (--array_elements);
		return sum;
	}
#else
	__m512dd vsum(size_t array_elements, const __m512dd array[restrict static array_elements]) {
		__m512dd sum = _mm512_setzero_pdd();
		do {
			sum = _mm512_add_pdd(sum, *array++);
		} while (--array_elements);
		return sum;
	}
#endif

/* Chained product of array elements - benchmark for multiplication latency */
#ifndef __KNC__
	doubledouble vprod(size_t array_elements, const doubledouble array[restrict static array_elements]) {
		doubledouble prod = { 1.0, 0.0 };
		do {
			prod = ddmul(prod, *array++);
		} while (--array_elements);
		return prod;
	}
#else
	__m512dd vprod(size_t array_elements, const __m512dd array[restrict static array_elements]) {
		__m512dd prod = _mm512_setzero_pdd();
		do {
			prod = _mm512_mul_pdd(prod, *array++);
		} while (--array_elements);
		return prod;
	}
#endif

/* Addition of a constant to an array - benchmark for addition throughput */
#ifndef __KNC__
	void vaddc(size_t augend_elements, doubledouble augend[restrict static augend_elements], const doubledouble addend) {
		for (size_t i = 0; i < augend_elements; i++) {
			augend[i] = ddadd(augend[i], addend);
		}
	}
#else
	void vaddc(size_t augend_elements, __m512dd augend[restrict static augend_elements], const __m512dd addend) {
		/* Xeon Phi is in-order, so it needs explicitly unrolled loop to extract ILP */
		for (size_t i = 0; i < augend_elements; i += 2) {
			augend[i] = _mm512_add_pdd(augend[i], addend);
			augend[i+1] = _mm512_add_pdd(augend[i+1], addend);
		}
	}
#endif

/* Multiplication of an array by a constant - benchmark for multiplication throughput */
#ifndef __KNC__
	void vmulc(size_t multiplicand_elements, doubledouble multiplicand[restrict static multiplicand_elements], const doubledouble multiplier) {
		for (size_t i = 0; i < multiplicand_elements; i++) {
			multiplicand[i] = ddmul(multiplicand[i], multiplier);
		}
	}
#else
	void vmulc(size_t multiplicand_elements, __m512dd multiplicand[restrict static multiplicand_elements], const __m512dd multiplier) {
		/* Xeon Phi is in-order, so it needs explicitly unrolled loop to extract ILP */
		for (size_t i = 0; i < multiplicand_elements; i += 2) {
			multiplicand[i] = _mm512_mul_pdd(multiplicand[i], multiplier);
			multiplicand[i+1] = _mm512_mul_pdd(multiplicand[i+1], multiplier);
		}
	}
#endif

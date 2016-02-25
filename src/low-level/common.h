#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <math.h>
#include <fpplus.h>
#ifdef FPPLUS_HAVE_FLOAT128
#include <quadmath.h>
#endif


enum benchmark_type {
	benchmark_type_none = 0,
	benchmark_type_doubledouble_latency,
	benchmark_type_doubledouble_throughput,
#ifdef FPPLUS_HAVE_FLOAT128
	benchmark_type_quad_latency,
#endif
	benchmark_type_polevl_latency,
};

struct benchmark_options {
	enum benchmark_type type;
	size_t iterations;
	size_t repeats;
};

struct benchmark_options parse_options(int argc, char** argv);


/* Benchmarks of double-double precision operations */
#ifndef __KNC__
	typedef doubledouble (*benchmark_doubledouble_function)(size_t, doubledouble*restrict);
	
	doubledouble vsum(size_t array_elements, const doubledouble array[restrict static array_elements]);
	doubledouble vprod(size_t array_elements, const doubledouble array[restrict static array_elements]);
	void vaddc(size_t augend_elements, doubledouble augend[restrict static augend_elements], const doubledouble addend);
	void vmulc(size_t multiplicand_elements, doubledouble multiplicand[restrict static multiplicand_elements], const doubledouble multiplier);

	inline static doubledouble vaddc_helper(size_t array_elements, doubledouble array[restrict static array_elements]) {
		vaddc(array_elements, array, (doubledouble) { M_E, M_PI });
		return (doubledouble) { 0.0, 0.0 };
	}

	inline static doubledouble vmulc_helper(size_t array_elements, doubledouble array[restrict static array_elements]) {
		vmulc(array_elements, array, (doubledouble) { M_E, M_PI });
		return (doubledouble) { 0.0, 0.0 };
	}
#else
	typedef __m512dd (*benchmark_doubledouble_function)(size_t, __m512dd*restrict);
	
	__m512dd vsum(size_t array_elements, const __m512dd array[restrict static array_elements]);
	__m512dd vprod(size_t array_elements, const __m512dd array[restrict static array_elements]);
	void vaddc(size_t augend_elements, __m512dd augend[restrict static augend_elements], const __m512dd addend);
	void vmulc(size_t multiplicand_elements, __m512dd multiplicand[restrict static multiplicand_elements], const __m512dd multiplier);

	inline static __m512dd vaddc_helper(size_t array_elements, __m512dd array[restrict static array_elements]) {
		vaddc(array_elements, array, (__m512dd) { _mm512_set1_pd(M_E), _mm512_set1_pd(M_PI) });
		return _mm512_setzero_pdd();
	}

	inline static __m512dd vmulc_helper(size_t array_elements, __m512dd array[restrict static array_elements]) {
		vmulc(array_elements, array, (__m512dd) { _mm512_set1_pd(M_E), _mm512_set1_pd(M_PI) });
		return _mm512_setzero_pdd();
	}
#endif

#ifdef FPPLUS_HAVE_FLOAT128
	/* Benchmarks of quad-precision operations */
	typedef __float128 (*benchmark_quad_function)(size_t, const __float128*restrict);

	__float128 qsum(size_t array_elements, const __float128 array[restrict static array_elements]);
	__float128 qprod(size_t array_elements, const __float128 array[restrict static array_elements]);
#endif


/* Benchmarks of polynomial evaluation latency */
#ifndef __KNC__
	typedef double (*benchmark_polevl_function)(double, size_t);

	double benchmark_compensated_horner15(double x, size_t iterations);
	double benchmark_muladd_horner15(double x, size_t iterations);
	double benchmark_fma_horner15(double x, size_t iterations);
#else
	typedef __m512d (*benchmark_polevl_function)(__m512d, size_t);

	__m512d benchmark_compensated_horner15(__m512d x, size_t iterations);
	__m512d benchmark_muladd_horner15(__m512d x, size_t iterations);
	__m512d benchmark_fma_horner15(__m512d x, size_t iterations);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

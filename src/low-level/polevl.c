#include <low-level/common.h>

#include <fpplus/polevl.h>

/* Benchmarks for latency of polinomial evalution with Horner scheme */

/* Polynomial evaluation with compensated Horner scheme */
#ifndef __KNC__
double benchmark_compensated_horner15(double x, size_t iterations) {
#else
__m512d benchmark_compensated_horner15(__m512d x, size_t iterations) {
#endif
	const double c0 = 0x1.78f187ab028a6p-1;
	const double c1 = 0x1.3f5db1c895000p-11;
	const double c2 = 0x1.7a26b65c2b4f0p-3;
	const double c3 = 0x1.bf60f17a47170p-3;
	const double c4 = 0x1.9aab2397bc0cdp-1;
	const double c5 = 0x1.e29e3de72e176p-2;
	const double c6 = 0x1.ecbb9a473c660p-5;
	const double c7 = 0x1.697d6c1218d5ep-1;
	const double c8 = 0x1.dd0cb5bd6c8c6p-2;
	const double c9 = 0x1.335b4defa4ac0p-7;
	const double c10 = 0x1.7bb63e1392fe5p-1;
	const double c11 = 0x1.03785a13a5632p-1;
	const double c12 = 0x1.ab7eb01482804p-2;
	const double c13 = 0x1.f867317158ce0p-3;
	const double c14 = 0x1.2fb1b3016c6e2p-2;
	const double c15 = 0x1.cda91c1ea93d0p-3;
	do {
		#ifndef __KNC__
			x = complensated_horner15(x, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15);
		#else
			x = _mm512_comp_horner15_pd(x, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15);
		#endif
	} while (--iterations);
	return x;
}

/* Polynomial evaluation with Horner scheme with multiplication and addition involving intermediate rounding */
#ifndef __KNC__
double benchmark_muladd_horner15(double x, size_t iterations) {
#else
__m512d benchmark_muladd_horner15(__m512d x, size_t iterations) {
#endif
	const double c0 = 0x1.78f187ab028a6p-1;
	const double c1 = 0x1.3f5db1c895000p-11;
	const double c2 = 0x1.7a26b65c2b4f0p-3;
	const double c3 = 0x1.bf60f17a47170p-3;
	const double c4 = 0x1.9aab2397bc0cdp-1;
	const double c5 = 0x1.e29e3de72e176p-2;
	const double c6 = 0x1.ecbb9a473c660p-5;
	const double c7 = 0x1.697d6c1218d5ep-1;
	const double c8 = 0x1.dd0cb5bd6c8c6p-2;
	const double c9 = 0x1.335b4defa4ac0p-7;
	const double c10 = 0x1.7bb63e1392fe5p-1;
	const double c11 = 0x1.03785a13a5632p-1;
	const double c12 = 0x1.ab7eb01482804p-2;
	const double c13 = 0x1.f867317158ce0p-3;
	const double c14 = 0x1.2fb1b3016c6e2p-2;
	const double c15 = 0x1.cda91c1ea93d0p-3;
	do {
		#ifndef __KNC__
			x = muladd_horner15(x, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15);
		#else
			x = _mm512_muladd_horner15_pd(x, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15);
		#endif
	} while (--iterations);
	return x;
}

/* Polynomial evaluation with Horner scheme with fused multiply-add */
#ifndef __KNC__
double benchmark_fma_horner15(double x, size_t iterations) {
#else
__m512d benchmark_fma_horner15(__m512d x, size_t iterations) {
#endif
	const double c0 = 0x1.78f187ab028a6p-1;
	const double c1 = 0x1.3f5db1c895000p-11;
	const double c2 = 0x1.7a26b65c2b4f0p-3;
	const double c3 = 0x1.bf60f17a47170p-3;
	const double c4 = 0x1.9aab2397bc0cdp-1;
	const double c5 = 0x1.e29e3de72e176p-2;
	const double c6 = 0x1.ecbb9a473c660p-5;
	const double c7 = 0x1.697d6c1218d5ep-1;
	const double c8 = 0x1.dd0cb5bd6c8c6p-2;
	const double c9 = 0x1.335b4defa4ac0p-7;
	const double c10 = 0x1.7bb63e1392fe5p-1;
	const double c11 = 0x1.03785a13a5632p-1;
	const double c12 = 0x1.ab7eb01482804p-2;
	const double c13 = 0x1.f867317158ce0p-3;
	const double c14 = 0x1.2fb1b3016c6e2p-2;
	const double c15 = 0x1.cda91c1ea93d0p-3;
	do {
		#ifndef __KNC__
			x = fma_horner15(x, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15);
		#else
			x = _mm512_fma_horner15_pd(x, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15);
		#endif
	} while (--iterations);
	return x;
}

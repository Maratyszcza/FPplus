#pragma once

#include <cstddef>
#include <cstdlib>

#include <cmath>
#include <cfloat>
#include <limits>
#include <vector>
#include <random>
#include <chrono>
#include <functional>
#include <algorithm>

#include <mpfr.h>

#include <gtest/gtest.h>

#include <fpplus.h>

#include <ddgemm/ddgemm.h>


template<size_t mrT, size_t nrT, size_t simdWidthT, ddgemm_function FunctionT>
class DDGEMMTester {
public:
	DDGEMMTester() :
		errorLimit_(1.0e-30)
	{
	}

	DDGEMMTester(const DDGEMMTester&) = delete;

	DDGEMMTester& operator=(const DDGEMMTester&) = delete;

	DDGEMMTester& errorLimit(double errorLimit) {
		this->errorLimit_ = errorLimit;
		return *this;
	}

	double errorLimit() const {
		return this->errorLimit_;
	}

	void test(size_t kc = 1024) const {
		const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
		auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));

		mpfr_t mp_a, mp_b, mp_error, mp_acc[mrT][nrT];
		for (size_t m = 0; m < mrT; m++) {
			for (size_t n = 0; n < nrT; n++) {
				mpfr_init2(mp_acc[m][n], DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
				mpfr_set_zero(mp_acc[m][n], 0);
			}
		}

		double* array_a = (double*) valloc(mrT * kc * sizeof(doubledouble));
		doubledouble* array_b = (doubledouble*) valloc(nrT * kc * sizeof(doubledouble));
		doubledouble* array_c = (doubledouble*) valloc(mrT * nrT * sizeof(doubledouble));
		memset(array_c, 0, mrT * nrT * sizeof(doubledouble));

		mpfr_init2(mp_a, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
		mpfr_init2(mp_b, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
		for (size_t k = 0; k < kc; k++) {
			for (size_t m = 0; m < mrT; m++) {
				doubledouble a = { rng(), DBL_EPSILON * rng() };
				a.hi = efaddord(a.hi, a.lo, &a.lo);

				array_a[2 * k * mrT + (m / simdWidthT) * (2 * simdWidthT) + m % simdWidthT] = a.hi;
				array_a[2 * k * mrT + (m / simdWidthT) * (2 * simdWidthT) + simdWidthT + m % simdWidthT] = a.lo;
			}
			for (size_t n = 0; n < nrT; n++) {
				doubledouble b = { rng(), DBL_EPSILON * rng() };
				b.hi = efaddord(b.hi, b.lo, &b.lo);

				array_b[k * nrT + n] = b;
			}
			for (size_t m = 0; m < mrT; m++) {
				for (size_t n = 0; n < nrT; n++) {
					mpfr_set_d(mp_a, array_a[2 * k * mrT + (m / simdWidthT) * (2 * simdWidthT) + m % simdWidthT], MPFR_RNDN);
					mpfr_add_d(mp_a, mp_a, array_a[2 * k * mrT + (m / simdWidthT) * (2 * simdWidthT) + simdWidthT + m % simdWidthT], MPFR_RNDN);

					mpfr_set_d(mp_b, array_b[k * nrT + n].hi, MPFR_RNDN);
					mpfr_add_d(mp_b, mp_b, array_b[k * nrT + n].lo, MPFR_RNDN);

					mpfr_fma(mp_acc[m][n], mp_a, mp_b, mp_acc[m][n], MPFR_RNDN);
				}
			}
		}
		mpfr_clear(mp_a);
		mpfr_clear(mp_b);

		FunctionT(kc, array_a, array_b, array_c);

		mpfr_init2(mp_error, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
		for (size_t m = 0; m < mrT; m++) {
			for (size_t n = 0; n < nrT; n++) {
				mpfr_sub_d(mp_error, mp_acc[m][n], array_c[n * mrT + m].hi, MPFR_RNDN);
				mpfr_sub_d(mp_error, mp_error, array_c[n * mrT + m].lo, MPFR_RNDN);
				mpfr_div(mp_error, mp_error, mp_acc[m][n], MPFR_RNDN);
				mpfr_clear(mp_acc[m][n]);

				const double error = mpfr_get_d(mp_error, MPFR_RNDN);
				EXPECT_LT(error, errorLimit()) <<
					"C[" << m << "][" << n << "] error is " << error;
			}
		}
		mpfr_clear(mp_error);

		free(array_a);
		free(array_b);
		free(array_c);
	}

private:
	double errorLimit_;
};

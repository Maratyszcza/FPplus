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

#include <dot/dot.h>


class DotTester {
public:
	DotTester() :
		arrayElements_(1027),
		a(nullptr),
		b(nullptr)
	{
		mpfr_init2(mp_tmp, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
		mpfr_init2(mp_sum, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);

		this->resize();
	}

	DotTester(const DotTester&) = delete;

	DotTester& operator=(const DotTester&) = delete;

	~DotTester() {
		mpfr_clear(this->mp_tmp);
		mpfr_clear(this->mp_sum);
		free(this->a);
		free(this->b);
	}

	DotTester& arrayElements(size_t arrayElements) {
		this->arrayElements_ = arrayElements;
		this->resize();
		return *this;
	}

	size_t arrayElements() const {
		return this->arrayElements_;
	}

	void testDotProduct(dot_product_function dotProduct, double errorLimit = 5.0 * DBL_EPSILON) {
		this->regenerateArrays();
		this->recomputeReference();

		double sum = dotProduct(arrayElements(), this->a, this->b);
		mpfr_sub_d(mp_tmp, mp_sum, sum, MPFR_RNDN);
		mpfr_div(mp_tmp, mp_tmp, mp_sum, MPFR_RNDN);

		const double relativeError = fabs(mpfr_get_d(mp_tmp, MPFR_RNDN));
		ASSERT_LT(relativeError, errorLimit);
	}

	void testCompensatedDotProduct(
		compensated_dot_product_function compensatedDotProduct,
		double errorLimit = 10.0 * DBL_EPSILON * DBL_EPSILON)
	{
		this->regenerateArrays();
		this->recomputeReference();

		doubledouble sum = compensatedDotProduct(arrayElements(), this->a, this->b);
		mpfr_sub_d(mp_tmp, mp_sum, sum.hi, MPFR_RNDN);
		mpfr_sub_d(mp_tmp, mp_tmp, sum.lo, MPFR_RNDN);
		mpfr_div(mp_tmp, mp_tmp, mp_sum, MPFR_RNDN);

		const double relativeError = fabs(mpfr_get_d(mp_tmp, MPFR_RNDN));
		ASSERT_LT(relativeError, errorLimit);
	}

private:
	/**
	 * @brief Rellocates @b a and @b b arrays according to arrayElements() value.
	 */
	void resize() {
		free(this->a);
		free(this->b);
		this->a = static_cast<double*>(valloc(arrayElements() * sizeof(double)));
		this->b = static_cast<double*>(valloc(arrayElements() * sizeof(double)));
	}

	/**
	 * @brief (Re-)initializes @b a and @b b arrays with random numbers
	 */
	void regenerateArrays() {
		const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
		auto rng = std::bind(std::uniform_real_distribution<double>(-1.0, 1.0), std::mt19937(seed));
		std::generate(this->a, this->a + arrayElements(), rng);
		std::generate(this->b, this->b + arrayElements(), rng);
	}

	/**
	 * @brief Recomputes the high-precision value of dot product in @b mp_sum.
	 */
	void recomputeReference() {
		mpfr_set_zero(mp_sum, 0);
		for (size_t i = 0; i < arrayElements(); i++) {
			mpfr_set_d(mp_tmp, this->a[i], MPFR_RNDN);
			mpfr_mul_d(mp_tmp, mp_tmp, this->b[i], MPFR_RNDN);
			mpfr_add(mp_sum, mp_sum, mp_tmp, MPFR_RNDN);
		}
	}

	size_t arrayElements_;
	mutable double* a;
	mutable double* b;
	mutable mpfr_t mp_tmp;
	mutable mpfr_t mp_sum;
};

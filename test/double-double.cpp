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

/* Check that the high double is the sum of addends rounded to closest double-precision number */
TEST(ddaddl, high_double) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double a = rng();
		const double b = rng();
		const doubledouble sum = ddaddl(a, b);
		EXPECT_EQ(sum.hi, a + b) << "a = " << a << " b = " << b;
	}
}

/* Check that the result is at least as accuate as double-precision addition when addends have the same sign */
TEST(ddaddl, same_sign_accuracy) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	mpfr_t mp_sum_a_b, mp_sum_hi_lo, mp_error_doubledouble, mp_error_double;
	mpfr_init2(mp_sum_a_b, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_sum_hi_lo, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_doubledouble, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_double, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double a = rng();
		const double b = rng();
		const doubledouble sum = ddaddl(a, b);

		mpfr_set_d(mp_sum_a_b, a, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, b, MPFR_RNDN);

		mpfr_set_d(mp_sum_hi_lo, sum.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_hi_lo, mp_sum_hi_lo, sum.lo, MPFR_RNDN);

		mpfr_sub(mp_error_doubledouble, mp_sum_a_b, mp_sum_hi_lo, MPFR_RNDN);
		mpfr_sub_d(mp_error_double, mp_sum_a_b, a + b, MPFR_RNDN);

		EXPECT_LE(mpfr_cmpabs(mp_error_doubledouble, mp_error_double), 0) << "a = " << a << " b = " << b;
	}
	mpfr_clear(mp_sum_a_b);
	mpfr_clear(mp_sum_hi_lo);
	mpfr_clear(mp_error_doubledouble);
	mpfr_clear(mp_error_double);
}

/* Check that the result is at least as accuate as double-precision addition when addends have opposite signs */
TEST(ddaddl, opposite_sign_accuracy) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	mpfr_t mp_sum_a_b, mp_sum_hi_lo, mp_error_doubledouble, mp_error_double;
	mpfr_init2(mp_sum_a_b, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_sum_hi_lo, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_doubledouble, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_double, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double a =  rng();
		const double b = -rng();
		const doubledouble sum = ddaddl(a, b);

		mpfr_set_d(mp_sum_a_b, a, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, b, MPFR_RNDN);

		mpfr_set_d(mp_sum_hi_lo, sum.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_hi_lo, mp_sum_hi_lo, sum.lo, MPFR_RNDN);

		mpfr_sub(mp_error_doubledouble, mp_sum_a_b, mp_sum_hi_lo, MPFR_RNDN);
		mpfr_sub_d(mp_error_double, mp_sum_a_b, a + b, MPFR_RNDN);

		EXPECT_LE(mpfr_cmpabs(mp_error_doubledouble, mp_error_double), 0) << "a = " << a << " b = " << b;
	}
	mpfr_clear(mp_sum_a_b);
	mpfr_clear(mp_sum_hi_lo);
	mpfr_clear(mp_error_doubledouble);
	mpfr_clear(mp_error_double);
}

/* Check that the result is at least as accuate as double-precision addition when addends have the same sign */
TEST(ddaddw, same_sign_accuracy) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	mpfr_t mp_sum_a_b, mp_sum_hi_lo, mp_error_doubledouble, mp_error_double;
	mpfr_init2(mp_sum_a_b, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_sum_hi_lo, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_doubledouble, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_double, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		/* Generate random normalized double-double number */
		doubledouble a = { rng(), rng() * DBL_EPSILON };
		a.hi = efaddord(a.hi, a.lo, &a.lo);

		const double b = rng();
		const doubledouble sum = ddaddw(a, b);

		mpfr_set_d(mp_sum_a_b, a.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, a.lo, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, b, MPFR_RNDN);

		mpfr_set_d(mp_sum_hi_lo, sum.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_hi_lo, mp_sum_hi_lo, sum.lo, MPFR_RNDN);

		mpfr_sub(mp_error_doubledouble, mp_sum_a_b, mp_sum_hi_lo, MPFR_RNDN);
		mpfr_sub_d(mp_error_double, mp_sum_a_b, (a.hi + b) + a.lo, MPFR_RNDN);

		EXPECT_LE(mpfr_cmpabs(mp_error_doubledouble, mp_error_double), 0) << "a = " << a.hi << " + " << a.lo << " b = " << b;
	}
	mpfr_clear(mp_sum_a_b);
	mpfr_clear(mp_sum_hi_lo);
	mpfr_clear(mp_error_doubledouble);
	mpfr_clear(mp_error_double);
}

/* Check that the result is at least as accuate as double-precision addition when addends have opposite signs */
TEST(ddaddw, opposite_sign_accuracy) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	mpfr_t mp_sum_a_b, mp_sum_hi_lo, mp_error_doubledouble, mp_error_double;
	mpfr_init2(mp_sum_a_b, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_sum_hi_lo, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_doubledouble, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_double, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		/* Generate random normalized double-double number */
		doubledouble a = { rng(), rng() * DBL_EPSILON };
		a.hi = efaddord(a.hi, a.lo, &a.lo);

		const double b = -rng();
		const doubledouble sum = ddaddw(a, b);

		mpfr_set_d(mp_sum_a_b, a.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, a.lo, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, b, MPFR_RNDN);

		mpfr_set_d(mp_sum_hi_lo, sum.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_hi_lo, mp_sum_hi_lo, sum.lo, MPFR_RNDN);

		mpfr_sub(mp_error_doubledouble, mp_sum_a_b, mp_sum_hi_lo, MPFR_RNDN);
		mpfr_sub_d(mp_error_double, mp_sum_a_b, (a.hi + b) + a.lo, MPFR_RNDN);

		EXPECT_LE(mpfr_cmpabs(mp_error_doubledouble, mp_error_double), 0) << "a = " << a.hi << " + " << a.lo << " b = " << b;
	}
	mpfr_clear(mp_sum_a_b);
	mpfr_clear(mp_sum_hi_lo);
	mpfr_clear(mp_error_doubledouble);
	mpfr_clear(mp_error_double);
}

/* Check that the result is at least as accuate as double-precision addition when addends have the same sign */
TEST(ddadd, same_sign_accuracy) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	mpfr_t mp_sum_a_b, mp_sum_hi_lo, mp_error_doubledouble, mp_error_double;
	mpfr_init2(mp_sum_a_b, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_sum_hi_lo, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_doubledouble, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_double, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		/* Generate random normalized double-double numbers */
		doubledouble a = { rng(), rng() * DBL_EPSILON };
		doubledouble b = { rng(), rng() * DBL_EPSILON };
		a.hi = efaddord(a.hi, a.lo, &a.lo);
		b.hi = efaddord(b.hi, b.lo, &b.lo);

		const doubledouble sum = ddadd(a, b);

		mpfr_set_d(mp_sum_a_b, a.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, a.lo, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, b.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, b.lo, MPFR_RNDN);

		mpfr_set_d(mp_sum_hi_lo, sum.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_hi_lo, mp_sum_hi_lo, sum.lo, MPFR_RNDN);

		mpfr_sub(mp_error_doubledouble, mp_sum_a_b, mp_sum_hi_lo, MPFR_RNDN);
		mpfr_sub_d(mp_error_double, mp_sum_a_b, (a.hi + b.hi) + (a.lo + b.lo), MPFR_RNDN);

		EXPECT_LE(mpfr_cmpabs(mp_error_doubledouble, mp_error_double), 0) <<
			"a = " << a.hi << " + " << a.lo << " b = " << b.hi << " + " << b.lo;
	}
	mpfr_clear(mp_sum_a_b);
	mpfr_clear(mp_sum_hi_lo);
	mpfr_clear(mp_error_doubledouble);
	mpfr_clear(mp_error_double);
}

/* Check that the result is at least as accuate as double-precision addition when addends have opposite signs */
TEST(ddadd, opposite_sign_accuracy) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	mpfr_t mp_sum_a_b, mp_sum_hi_lo, mp_error_doubledouble, mp_error_double;
	mpfr_init2(mp_sum_a_b, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_sum_hi_lo, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_doubledouble, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_double, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		/* Generate random normalized double-double numbers */
		doubledouble a = { +rng(), +rng() * DBL_EPSILON };
		doubledouble b = { -rng(), -rng() * DBL_EPSILON };
		a.hi = efaddord(a.hi, a.lo, &a.lo);
		b.hi = efaddord(b.hi, b.lo, &b.lo);

		const doubledouble sum = ddadd(a, b);

		mpfr_set_d(mp_sum_a_b, a.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, a.lo, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, b.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, b.lo, MPFR_RNDN);

		mpfr_set_d(mp_sum_hi_lo, sum.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_hi_lo, mp_sum_hi_lo, sum.lo, MPFR_RNDN);

		mpfr_sub(mp_error_doubledouble, mp_sum_a_b, mp_sum_hi_lo, MPFR_RNDN);
		mpfr_sub_d(mp_error_double, mp_sum_a_b, (a.hi + b.hi) + (a.lo + b.lo), MPFR_RNDN);

		EXPECT_LE(mpfr_cmpabs(mp_error_doubledouble, mp_error_double), 0) <<
			"a = " << a.hi << " + " << a.lo << " b = " << b.hi << " + " << b.lo;
	}
	mpfr_clear(mp_sum_a_b);
	mpfr_clear(mp_sum_hi_lo);
	mpfr_clear(mp_error_doubledouble);
	mpfr_clear(mp_error_double);
}

/* Check that the result is at least as accuate as double-precision addition when addends have the same sign */
TEST(ddadd_fast, same_sign_accuracy) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	mpfr_t mp_sum_a_b, mp_sum_hi_lo, mp_error_doubledouble, mp_error_double;
	mpfr_init2(mp_sum_a_b, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_sum_hi_lo, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_doubledouble, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_double, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		/* Generate random normalized double-double numbers */
		doubledouble a = { rng(), rng() * DBL_EPSILON };
		doubledouble b = { rng(), rng() * DBL_EPSILON };
		a.hi = efaddord(a.hi, a.lo, &a.lo);
		b.hi = efaddord(b.hi, b.lo, &b.lo);

		const doubledouble sum = ddadd_fast(a, b);

		mpfr_set_d(mp_sum_a_b, a.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, a.lo, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, b.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, b.lo, MPFR_RNDN);

		mpfr_set_d(mp_sum_hi_lo, sum.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_hi_lo, mp_sum_hi_lo, sum.lo, MPFR_RNDN);

		mpfr_sub(mp_error_doubledouble, mp_sum_a_b, mp_sum_hi_lo, MPFR_RNDN);
		mpfr_sub_d(mp_error_double, mp_sum_a_b, (a.hi + b.hi) + (a.lo + b.lo), MPFR_RNDN);

		EXPECT_LE(mpfr_cmpabs(mp_error_doubledouble, mp_error_double), 0) <<
			"a = " << a.hi << " + " << a.lo << " b = " << b.hi << " + " << b.lo;
	}
	mpfr_clear(mp_sum_a_b);
	mpfr_clear(mp_sum_hi_lo);
	mpfr_clear(mp_error_doubledouble);
	mpfr_clear(mp_error_double);
}

/* Check that the result is at least as accuate as double-precision addition when addends have opposite signs */
TEST(ddadd_fast, opposite_sign_accuracy) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	mpfr_t mp_sum_a_b, mp_sum_hi_lo, mp_error_doubledouble, mp_error_double;
	mpfr_init2(mp_sum_a_b, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_sum_hi_lo, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_doubledouble, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_double, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		/* Generate random normalized double-double numbers */
		doubledouble a = { +rng(), +rng() * DBL_EPSILON };
		doubledouble b = { -rng(), -rng() * DBL_EPSILON };
		a.hi = efaddord(a.hi, a.lo, &a.lo);
		b.hi = efaddord(b.hi, b.lo, &b.lo);

		const doubledouble sum = ddadd_fast(a, b);

		mpfr_set_d(mp_sum_a_b, a.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, a.lo, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, b.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_a_b, mp_sum_a_b, b.lo, MPFR_RNDN);

		mpfr_set_d(mp_sum_hi_lo, sum.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_hi_lo, mp_sum_hi_lo, sum.lo, MPFR_RNDN);

		mpfr_sub(mp_error_doubledouble, mp_sum_a_b, mp_sum_hi_lo, MPFR_RNDN);
		mpfr_sub_d(mp_error_double, mp_sum_a_b, (a.hi + b.hi) + (a.lo + b.lo), MPFR_RNDN);

		EXPECT_LE(mpfr_cmpabs(mp_error_doubledouble, mp_error_double), 0) <<
			"a = " << a.hi << " + " << a.lo << " b = " << b.hi << " + " << b.lo;
	}
	mpfr_clear(mp_sum_a_b);
	mpfr_clear(mp_sum_hi_lo);
	mpfr_clear(mp_error_doubledouble);
	mpfr_clear(mp_error_double);
}

/* Check that the high double is the product of factors rounded to closest double-precision number */
TEST(ddmull, high_double) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double a = rng();
		const double b = rng();
		const doubledouble prod = ddmull(a, b);
		EXPECT_EQ(prod.hi, a * b) << "a = " << a << " b = " << b;
	}
}

/* Check that the result is at least as accuate as double-precision multiplication */
TEST(ddmull, accuracy) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	mpfr_t mp_prod_a_b, mp_sum_hi_lo, mp_error_doubledouble, mp_error_double;
	mpfr_init2(mp_prod_a_b, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_sum_hi_lo, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_doubledouble, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_double, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double a = rng();
		const double b = rng();
		const doubledouble sum = ddmull(a, b);

		mpfr_set_d(mp_prod_a_b, a, MPFR_RNDN);
		mpfr_mul_d(mp_prod_a_b, mp_prod_a_b, b, MPFR_RNDN);

		mpfr_set_d(mp_sum_hi_lo, sum.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_hi_lo, mp_sum_hi_lo, sum.lo, MPFR_RNDN);

		mpfr_sub(mp_error_doubledouble, mp_prod_a_b, mp_sum_hi_lo, MPFR_RNDN);
		mpfr_sub_d(mp_error_double, mp_prod_a_b, a * b, MPFR_RNDN);

		EXPECT_LE(mpfr_cmpabs(mp_error_doubledouble, mp_error_double), 0) << "a = " << a << " b = " << b;
	}
	mpfr_clear(mp_prod_a_b);
	mpfr_clear(mp_sum_hi_lo);
	mpfr_clear(mp_error_doubledouble);
	mpfr_clear(mp_error_double);
}

/* Check that the result is at least as accuate as double-precision multiplication */
TEST(ddmul, accuracy) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	mpfr_t mp_a, mp_b, mp_prod_a_b, mp_sum_hi_lo, mp_error_doubledouble, mp_error_double;
	mpfr_init2(mp_a, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_b, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_prod_a_b, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_sum_hi_lo, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_doubledouble, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(mp_error_double, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		/* Generate random normalized double-double numbers */
		doubledouble a = { rng(), rng() * DBL_EPSILON };
		doubledouble b = { rng(), rng() * DBL_EPSILON };
		a.hi = efaddord(a.hi, a.lo, &a.lo);
		b.hi = efaddord(b.hi, b.lo, &b.lo);

		const doubledouble prod = ddmul(a, b);

		mpfr_set_d(mp_a, a.hi, MPFR_RNDN);
		mpfr_add_d(mp_a, mp_a, a.lo, MPFR_RNDN);

		mpfr_set_d(mp_b, b.hi, MPFR_RNDN);
		mpfr_add_d(mp_b, mp_b, b.lo, MPFR_RNDN);

		mpfr_mul(mp_prod_a_b, mp_a, mp_b, MPFR_RNDN);

		mpfr_set_d(mp_sum_hi_lo, prod.hi, MPFR_RNDN);
		mpfr_add_d(mp_sum_hi_lo, mp_sum_hi_lo, prod.lo, MPFR_RNDN);

		mpfr_sub(mp_error_doubledouble, mp_prod_a_b, mp_sum_hi_lo, MPFR_RNDN);
		mpfr_sub_d(mp_error_double, mp_prod_a_b, a.hi * b.hi + ((a.lo * b.hi + a.hi * b.lo) + a.lo * b.lo), MPFR_RNDN);

		EXPECT_LE(mpfr_cmpabs(mp_error_doubledouble, mp_error_double), 0) <<
			"a = " << a.hi << " + " << a.lo << " b = " << b.hi << " + " << b.lo;
	}
	mpfr_clear(mp_a);
	mpfr_clear(mp_b);
	mpfr_clear(mp_prod_a_b);
	mpfr_clear(mp_sum_hi_lo);
	mpfr_clear(mp_error_doubledouble);
	mpfr_clear(mp_error_double);
}

int main(int ac, char* av[]) {
	testing::InitGoogleTest(&ac, av);
	return RUN_ALL_TESTS();
}

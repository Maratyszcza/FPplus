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

inline double ulp(double x) {
	x = fabs(x);
	return nextafter(x, std::numeric_limits<double>::infinity()) - x;
}

/* Check that the high double is the sum of addends rounded to closest double-precision number */
TEST(efadd, high_double) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double a = rng();
		const double b = rng();
		double e;
		const double sum = efadd(a, b, &e);
		EXPECT_EQ(sum, a + b) << "a = " << a << " b = " << b;
	}
}

/* Check that the low double is not greater than half ULP of the high double */
TEST(efadd, low_double) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double a = rng();
		const double b = rng();
		double e;
		const double sum = efadd(a, b, &e);
		EXPECT_LE(fabs(e), 0.5 * ulp(sum)) << "a = " << a << " b = " << b;
	}
}

/* Check that the sum of outputs equals the sum of inputs when inputs have the same sign */
TEST(efadd, same_sign_error_free) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));

	mpfr_t sum_ab, sum_se;
	mpfr_init2(sum_ab, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(sum_se, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double a = rng();
		const double b = rng();
		double e;
		const double s = efadd(a, b, &e);

		mpfr_set_d(sum_ab, a, MPFR_RNDN);
		mpfr_add_d(sum_ab, sum_ab, b, MPFR_RNDN);

		mpfr_set_d(sum_se, s, MPFR_RNDN);
		mpfr_add_d(sum_se, sum_se, e, MPFR_RNDN);

		EXPECT_TRUE(mpfr_equal_p(sum_ab, sum_se)) << "a = " << a << " b = " << b;
	}
	mpfr_clear(sum_ab);
	mpfr_clear(sum_se);
}

/* Check that the sum of outputs equals the sum of inputs when inputs have opposite signs */
TEST(efadd, opposite_sign_error_free) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));

	mpfr_t sum_ab, sum_se;
	mpfr_init2(sum_ab, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(sum_se, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double a = rng();
		const double b = -rng();
		double e;
		const double s = efadd(a, b, &e);

		mpfr_set_d(sum_ab, a, MPFR_RNDN);
		mpfr_add_d(sum_ab, sum_ab, b, MPFR_RNDN);

		mpfr_set_d(sum_se, s, MPFR_RNDN);
		mpfr_add_d(sum_se, sum_se, e, MPFR_RNDN);

		EXPECT_TRUE(mpfr_equal_p(sum_ab, sum_se)) << "a = " << a << " b = " << b;
	}
	mpfr_clear(sum_ab);
	mpfr_clear(sum_se);
}

/* Check that the high double is the sum of addends rounded to closest double-precision number */
TEST(efaddord, high_double) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double x = rng();
		const double y = rng();
		const double a = fmax(x, y);
		const double b = fmin(x, y);
		double e;
		const double sum = efaddord(a, b, &e);
		EXPECT_EQ(sum, a + b) << "a = " << a << " b = " << b;
	}
}

/* Check that the low double is not greater than half ULP of the high double */
TEST(efaddord, low_double) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double x = rng();
		const double y = rng();
		const double a = fmax(x, y);
		const double b = fmin(x, y);
		double e;
		const double sum = efaddord(a, b, &e);
		EXPECT_LE(fabs(e), 0.5 * ulp(sum)) << "a = " << a << " b = " << b;
	}
}

/* Check that the sum of outputs equals the sum of inputs when inputs have the same sign */
TEST(efaddord, same_sign_error_free) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));

	mpfr_t sum_ab, sum_se;
	mpfr_init2(sum_ab, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(sum_se, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double x = rng();
		const double y = rng();
		const double a = fmax(x, y);
		const double b = fmin(x, y);
		double e;
		const double s = efaddord(a, b, &e);

		mpfr_set_d(sum_ab, a, MPFR_RNDN);
		mpfr_add_d(sum_ab, sum_ab, b, MPFR_RNDN);

		mpfr_set_d(sum_se, s, MPFR_RNDN);
		mpfr_add_d(sum_se, sum_se, e, MPFR_RNDN);

		EXPECT_TRUE(mpfr_equal_p(sum_ab, sum_se)) << "a = " << a << " b = " << b;
	}
	mpfr_clear(sum_ab);
	mpfr_clear(sum_se);
}

/* Check that the sum of outputs equals the sum of inputs when inputs have opposite signs */
TEST(efaddord, opposite_sign_error_free) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));

	mpfr_t sum_ab, sum_se;
	mpfr_init2(sum_ab, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(sum_se, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double x = rng();
		const double y = rng();
		const double a = fmax(x, y);
		const double b = -fmin(x, y);
		double e;
		const double s = efaddord(a, b, &e);

		mpfr_set_d(sum_ab, a, MPFR_RNDN);
		mpfr_add_d(sum_ab, sum_ab, b, MPFR_RNDN);

		mpfr_set_d(sum_se, s, MPFR_RNDN);
		mpfr_add_d(sum_se, sum_se, e, MPFR_RNDN);

		EXPECT_TRUE(mpfr_equal_p(sum_ab, sum_se)) << "a = " << a << " b = " << b;
	}
	mpfr_clear(sum_ab);
	mpfr_clear(sum_se);
}

/* Check that the high double is the product of factors rounded to closest double-precision number */
TEST(efmul, high_double) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double a = rng();
		const double b = rng();
		double e;
		const double product = efmul(a, b, &e);
		EXPECT_EQ(product, a * b) << "a = " << a << " b = " << b;
	}
}

/* Check that the low double is not greater than half ULP of the high double */
TEST(efmul, low_double) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double a = rng();
		const double b = rng();
		double e;
		const double prod = efmul(a, b, &e);
		EXPECT_LE(fabs(e), 0.5 * ulp(prod)) << "a = " << a << " b = " << b;
	}
}

/* Check that the sum of outputs equals the sum of inputs */
TEST(efmul, error_free) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));

	mpfr_t prod_ab, sum_se;
	mpfr_init2(prod_ab, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(sum_se, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double a = rng();
		const double b = rng();
		double e;
		const double s = efmul(a, b, &e);

		mpfr_set_d(prod_ab, a, MPFR_RNDN);
		mpfr_mul_d(prod_ab, prod_ab, b, MPFR_RNDN);

		mpfr_set_d(sum_se, s, MPFR_RNDN);
		mpfr_add_d(sum_se, sum_se, e, MPFR_RNDN);

		EXPECT_TRUE(mpfr_equal_p(prod_ab, sum_se)) << "a = " << a << " b = " << b;
	}
	mpfr_clear(prod_ab);
	mpfr_clear(sum_se);
}

/* Check that the high double is the fma of inputs rounded to closest double-precision number */
TEST(effma, high_double) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double a = rng();
		const double b = rng();
		const double c = rng();
		double e_hi, e_lo;
		const double acc = effma(a, b, c, &e_hi, &e_lo);
		EXPECT_EQ(acc, fma(a, b, c)) << "a = " << a << " b = " << b << " c = " << c;
	}
}

/* Check that the error is not greater than half ULP of the high double */
TEST(effma, total_error) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double a = rng();
		const double b = rng();
		const double c = rng();
		double e_hi, e_lo;
		const double acc = effma(a, b, c, &e_hi, &e_lo);
		EXPECT_LE(fabs(e_lo + e_hi), 0.5 * ulp(acc)) << "a = " << a << " b = " << b << " c = " << c;
	}
}

/* Check that the high error is greater, in magnitude, than low error */
TEST(effma, high_low_error) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double a = rng();
		const double b = rng();
		const double c = rng();
		double e_hi, e_lo;
		effma(a, b, c, &e_hi, &e_lo);
		EXPECT_LE(fabs(e_lo), fabs(e_hi)) << "a = " << a << " b = " << b << " c = " << c;
	}
}

/* Check that the sum of outputs equals the sum of inputs */
TEST(effma, error_free) {
	const uint_fast32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::bind(std::uniform_real_distribution<double>(), std::mt19937(seed));

	mpfr_t acc_abc, sum_acc_error;
	mpfr_init2(acc_abc, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	mpfr_init2(sum_acc_error, DBL_MANT_DIG + DBL_MAX_EXP - DBL_MIN_EXP);
	for (size_t iteration = 0; iteration < 1000; iteration++) {
		const double a = rng();
		const double b = rng();
		const double c = rng();
		double e_hi, e_lo;
		const double acc = effma(a, b, c, &e_hi, &e_lo);

		MPFR_DECL_INIT(mp_a, DBL_MANT_DIG);
		MPFR_DECL_INIT(mp_b, DBL_MANT_DIG);
		MPFR_DECL_INIT(mp_c, DBL_MANT_DIG);
		mpfr_set_d(mp_a, a, MPFR_RNDN);
		mpfr_set_d(mp_b, b, MPFR_RNDN);
		mpfr_set_d(mp_c, c, MPFR_RNDN);

		mpfr_fma(acc_abc, mp_a, mp_b, mp_c, MPFR_RNDN);

		mpfr_set_d(sum_acc_error, acc, MPFR_RNDN);
		mpfr_add_d(sum_acc_error, sum_acc_error, e_hi, MPFR_RNDN);
		mpfr_add_d(sum_acc_error, sum_acc_error, e_lo, MPFR_RNDN);

		EXPECT_TRUE(mpfr_equal_p(acc_abc, sum_acc_error)) << "a = " << a << " b = " << b << " c = " << c;
	}
	mpfr_clear(acc_abc);
	mpfr_clear(sum_acc_error);
}

int main(int ac, char* av[]) {
	testing::InitGoogleTest(&ac, av);
	return RUN_ALL_TESTS();
}

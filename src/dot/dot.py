#!/usr/bin/env python

import sys
import os
import argparse

root_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(root_dir, ".."))

from code import CodeWriter, CodeBlock


parser = argparse.ArgumentParser(description="Dot product kernel generator")
parser.add_argument("--unroll-min", dest="unroll_min", required=True, type=int,
	help="Minimum unroll factor")
parser.add_argument("--unroll-max", dest="unroll_max", required=True, type=int,
	help="Maximum unroll factor")
parser.add_argument("--simd", dest="simd", choices=("scalar", "sse", "avx", "mic", "armv8", "vsx", "qpx"),
	help="SIMD intrinsics")
parser.add_argument("--implementation", dest="implementation", required=True,
	help="Output file name for C implementation")
parser.add_argument("--header", dest="header", required=True,
	help="Output file name for C/C++ header")
parser.add_argument("--unittest", dest="unittest", required=True,
	help="Output file name for C++ unit test")


def generate_dot_product(code, simd, unroll_factor, fma):
	code.line("""\
double dot_product_{fma_or_mac}_unroll{unroll_factor}(
	size_t n,
	const double a[restrict static n],
	const double b[restrict static n])
{{""".format(unroll_factor=unroll_factor, fma_or_mac="fma" if fma else "muladd"))
	with CodeBlock():
		for i in range(unroll_factor):
			code.line("{dvec} vsum{i} = {dzero};"
				.format(dvec=simd.dvec, dzero=simd.dzero(), i=i))
		code.line("for (; n >= {elements_per_loop}; n -= {elements_per_loop}) {{"
			.format(elements_per_loop=simd.width * unroll_factor))

		with CodeBlock() as vector_loop:
			for i in range(unroll_factor):
				code.line("const {dvec} va{i} = {dload}(a+{offset});"
					.format(dvec=simd.dvec, dload=simd._dload, i=i, offset=i*simd.width))
			for i in range(unroll_factor):
				code.line("const {dvec} vb{i} = {dload}(b+{offset});"
					.format(dvec=simd.dvec, dload=simd._dload, i=i, offset=i*simd.width))
			for i in range(unroll_factor):
				if fma:
					code.line("vsum{i} = {dfma}(va{i}, vb{i}, vsum{i});"
						.format(dvec=simd.dvec, dfma=simd._dfma, i=i))
				else:
					code.line("vsum{i} = {dadd}(vsum{i}, {dmul}(va{i}, vb{i}));"
						.format(dvec=simd.dvec, dadd=simd._dadd, dmul=simd._dmul, i=i))

			code.line("a += {elements_per_loop};".format(elements_per_loop=simd.width * unroll_factor))
			code.line("b += {elements_per_loop};".format(elements_per_loop=simd.width * unroll_factor))
		code.line("}")

		# Reduction of multiple SIMD vectors into a single SIMD vector
		reduction_offset = 1
		while reduction_offset <= unroll_factor:
			for i in range(0, unroll_factor - reduction_offset, 2 * reduction_offset):
				code.line("vsum{i} = {dadd}(vsum{i}, vsum{next_i});"
					.format(dadd=simd._dadd, i=i, next_i=i + reduction_offset))
			reduction_offset *= 2

		# Reduction of a SIMD vector into a scalar
		assert simd.name in ["avx", "mic"]
		if simd.name == "avx":
			code.line("double sum = _mm256_reduce_add_pd(vsum0);")
		elif simd.name == "mic":
			code.line("double sum = _mm512_reduce_add_pd(vsum0);")

		code.line("while (n--) {")
		with CodeBlock() as scalar_loop:
			if fma:
				code.line("#if defined(__GNUC__)")
				code.indent_line("sum = __builtin_fma(*a++, *b++, sum);")
				code.line("#else")
				code.indent_line("sum = fma(*a++, *b++, sum);")
				code.line("#endif")
			else:
				code.line("sum += (*a++) * (*b++);")
		code.line("}")
		code.line("return sum;");

	code.line("}")
	code.line()


def generate_compensated_dot_product(code, simd, unroll_factor):
	code.line("""
doubledouble compensated_dot_product_efmuladd_unroll{unroll_factor}(
	size_t n,
	const double a[restrict static n],
	const double b[restrict static n])
{{""".format(unroll_factor=unroll_factor))
	with CodeBlock():
		for i in range(unroll_factor):
			code.line("{ddvec} vsum{i} = {ddzero}();".format(ddvec=simd.ddvec, ddzero=simd.ddzero, i=i))
		code.line("for (; n>= {elements_per_loop}; n -= {elements_per_loop}) {{"
			.format(elements_per_loop=simd.width * unroll_factor))
		with CodeBlock():
			for index in range(unroll_factor):
				code.line("const {dvec} va{index} = {dload}(a+{offset});"
					.format(dvec=simd.dvec, dload=simd._dload, index=index, offset=index*simd.width))
			for index in range(unroll_factor):
				code.line("const {dvec} vb{index} = {dload}(b+{offset});"
					.format(dvec=simd.dvec, dload=simd._dload, index=index, offset=index*simd.width))
			for index in range(unroll_factor):
				code.line("{dvec} vproduct{index}_error, vsum{index}_error;"
					.format(dvec=simd.dvec, index=index))
			for index in range(unroll_factor):
				code.line("const {dvec} vproduct{index} = {defmul}(va{index}, vb{index}, &vproduct{index}_error);"
					.format(dvec=simd.dvec, defmul=simd._defmul, index=index))
			for index in range(unroll_factor):
				code.line("vsum{index}.hi = {defadd}(vsum{index}.hi, vproduct{index}, &vsum{index}_error);"
					.format(defadd=simd._defadd, index=index))
			for index in range(unroll_factor):
				code.line("vsum{index}.lo = {dadd}(vsum{index}.lo, {dadd}(vsum{index}_error, vproduct{index}_error));"
					.format(dadd=simd._dadd, index=index))
			code.line("a += {elements_per_loop};".format(elements_per_loop=simd.width * unroll_factor))
			code.line("b += {elements_per_loop};".format(elements_per_loop=simd.width * unroll_factor))
		code.line("}")

		# Reduction of multiple SIMD vectors into a single SIMD vector
		reduction_offset = 1
		while reduction_offset <= unroll_factor:
			for i in range(0, unroll_factor - reduction_offset, 2 * reduction_offset):
				code.line("vsum{i} = {ddadd}(vsum{i}, vsum{next_i});"
					.format(ddadd=simd._ddadd, i=i, next_i=i + reduction_offset))
			reduction_offset *= 2

		# Reduction of a SIMD vector into a scalar
		assert simd.name in ["avx", "mic"]
		if simd.name == "avx":
			code.line("doubledouble sum = _mm256_reduce_add_pdd(vsum0);")
		elif simd.name == "mic":
			code.line("doubledouble sum = _mm512_reduce_add_pdd(vsum0);")
		code.line("while (n--) {")
		with CodeBlock():
			code.line("double product_error, sum_error;")
			code.line("const double product = efmul(*a++, *b++, &product_error);")
			code.line("sum.hi = efadd(sum.hi, product, &sum_error);")
			code.line("sum.lo += (sum_error + product_error);")
		code.line("}")
		code.line("/* Normalize */")
		code.line("sum.hi = efaddord(sum.hi, sum.lo, &sum.lo);")
		code.line("return sum;");

	code.line("}")
	code.line()


def generate_dot_product_declaration(header, unroll_factor, implementation):
	header.line({
		"mac": "double dot_product_muladd_unroll{unroll_factor}(size_t n, const double a[], const double b[]);",
		"fma": "double dot_product_fma_unroll{unroll_factor}(size_t n, const double a[], const double b[]);",
		"compensated": "doubledouble compensated_dot_product_efmuladd_unroll{unroll_factor}(size_t n, const double a[], const double b[]);"
	}[implementation].format(unroll_factor=unroll_factor))


def generate_dot_product_unittest(unittest, unroll_factor, implementation):
	unittest.line("""\
TEST({operation}, {implementation}_unroll{unroll_factor}) {{
	DotTester()
		.test{test_method}({function}_unroll{unroll_factor});
}}
""".format(
	operation="compensated_dot_product" if implementation == "compensated" else "dot_product",
	implementation=implementation,
	unroll_factor=unroll_factor,
	test_method="CompensatedDotProduct" if implementation == "compensated" else "DotProduct",
	function={
		"mac": "dot_product_muladd",
		"fma": "dot_product_fma",
		"compensated": "compensated_dot_product_efmuladd"
	}[implementation]))


def main():
	options = parser.parse_args()

	from simd import SimdOperations
	with CodeWriter() as implementation:
		implementation.line("""\
#include <fpplus.h>

#include <dot/dot.h>
""")

		simd = SimdOperations(options.simd)
		if simd.name == "avx":
			implementation.line("""
FPPLUS_STATIC_INLINE double _mm_reduce_add_pd(const __m128d x) {
	const __m128d x_hi = _mm_unpackhi_pd(x, x);
	const __m128d sum = _mm_add_sd(x, x_hi);
	return _mm_cvtsd_f64(sum);
}

FPPLUS_STATIC_INLINE double _mm256_reduce_add_pd(const __m256d x) {
	const __m128d x_lo = _mm256_castpd256_pd128(x);
	const __m128d x_hi = _mm256_extractf128_pd(x, 1);
	return _mm_reduce_add_pd(_mm_add_pd(x_lo, x_hi));
}
""")

		for unroll_factor in range(options.unroll_min, options.unroll_max + 1):
			generate_dot_product(implementation, simd, unroll_factor, fma=False)

		for unroll_factor in range(options.unroll_min, options.unroll_max + 1):
			generate_dot_product(implementation, simd, unroll_factor, fma=True)

		for unroll_factor in range(options.unroll_min, options.unroll_max + 1):
			generate_compensated_dot_product(implementation, simd, unroll_factor)

	with CodeWriter() as header:
		header.line("""\
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include <fpplus.h>

typedef double (*dot_product_function)(size_t, const double*, const double*);
typedef doubledouble (*compensated_dot_product_function)(size_t, const double*, const double*);
""")

		header.line("/* Dot product based on multiplication and addition (with intermediate rounding) */")
		for unroll_factor in range(options.unroll_min, options.unroll_max + 1):
			generate_dot_product_declaration(header, unroll_factor, "mac")
		header.line()

		header.line("/* Dot product based on fused multiply-add */")
		for unroll_factor in range(options.unroll_min, options.unroll_max + 1):
			generate_dot_product_declaration(header, unroll_factor, "fma")
		header.line()

		header.line("/* compensated dot product based on error-free multiplication and error-free addition */")
		for unroll_factor in range(options.unroll_min, options.unroll_max + 1):
			generate_dot_product_declaration(header, unroll_factor, "compensated")

		header.line("""
#ifdef __cplusplus
} /* extern "C" */
#endif""")

	with CodeWriter() as unittest:
		unittest.line("""\
#include <cstddef>
#include <cstdlib>

#include <gtest/gtest.h>

#include <dot/dot.h>

#include "dot-tester.h"

""")

		unittest.line("/* Dot product based on multiplication and addition (with intermediate rounding) */")
		for unroll_factor in range(options.unroll_min, options.unroll_max + 1):
			generate_dot_product_unittest(unittest, unroll_factor, "mac")
		unittest.line()

		unittest.line("/* Dot product based on fused multiply-add */")
		for unroll_factor in range(options.unroll_min, options.unroll_max + 1):
			generate_dot_product_unittest(unittest, unroll_factor, "fma")
		unittest.line()

		unittest.line("/* compensated dot product based on error-free multiplication and error-free addition */")
		for unroll_factor in range(options.unroll_min, options.unroll_max + 1):
			generate_dot_product_unittest(unittest, unroll_factor, "compensated")

		unittest.line("""\
int main(int argc, char* argv[]) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
""")


	with open(options.implementation, "w") as implementation_file:
		implementation_file.write(str(implementation))

	with open(options.header, "w") as header_file:
		header_file.write(str(header))

	with open(options.unittest, "w") as unittest_file:
		unittest_file.write(str(unittest))


if __name__ == "__main__":
	sys.exit(main())

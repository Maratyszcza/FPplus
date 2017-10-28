#!/usr/bin/env python
from __future__ import division
        
import sys
import os
import argparse

root_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(root_dir, ".."))


parser = argparse.ArgumentParser(description="DDGEMM kernel generator")
parser.add_argument("--nr-min", dest="nr_min", required=True, type=int,
	help="Minimum register tiling of N dimension")
parser.add_argument("--nr-max", dest="nr_max", required=True, type=int,
	help="Maximum register tiling of N dimension")
parser.add_argument("--mr-min", dest="mr_min", required=True, type=int,
	help="Minimum register tiling of M dimension")
parser.add_argument("--mr-max", dest="mr_max", required=True, type=int,
	help="Maximum register tiling of M dimension")
parser.add_argument("--simd", dest="simd", required=True,
	choices=("scalar", "sse", "avx", "mic", "armv8", "vsx", "qpx"),
	help="SIMD intrinsics")
parser.add_argument("--implementation", dest="implementation", required=True,
	help="Output file name for C implementation")
parser.add_argument("--header", dest="header", required=True,
	help="Output file name for C/C++ header")
parser.add_argument("--unittest", dest="unittest", required=True,
	help="Output file name for C++ unit test")


def main():
	options = parser.parse_args()

	from code import CodeWriter, CodeBlock
	from simd import SimdOperations
	with CodeWriter() as impl:
		impl.line("#include <fpplus.h>")
		impl.line("#include <ddgemm/ddgemm.h>")
		impl.line()

		simd = SimdOperations(options.simd)
		for mr in range(options.mr_min, options.mr_max + 1, simd.width):
			for nr in range(options.nr_min, options.nr_max + 1):
				impl.line("""\
void ddgemm{mr}x{nr}(size_t k,
	const double a[restrict static 2*k*{mr}],
	const doubledouble b[restrict static k*{nr}],
	doubledouble c[restrict static {mr}*{nr}])
{{""".format(mr=mr, nr=nr))
				with CodeBlock():
					for m in range(mr // simd.width):
						impl.line("{ddvec} {vars};".
							format(ddvec=simd.ddvec,
								vars=", ".join("va{m}b{n} = {ddzero}()".format(m=m, n=n, ddzero=simd.ddzero) for n in range(nr))))

					impl.line("do {")
					with CodeBlock():
						for m in range(mr // simd.width):
							impl.line("const {ddvec} va{m} = {{ {dload}(a + {index_hi}), {dload}(a + {index_lo}) }};"
								.format(ddvec = simd.ddvec, m=m, dload=simd._dload,
									index_hi=(2*m)*simd.width, index_lo=(2*m+1)*simd.width))
						impl.line()

						for n in range(nr):
							impl.line("{ddvec} vb{n} = {ddbroadcast}(b+{n});".format(ddvec=simd.ddvec, ddbroadcast=simd.ddbroadcast, n=n))
							for m in range(mr // simd.width):
								vanbm = "va{m}b{n}".format(m=m, n=n)
								impl.line(vanbm + " = " + simd.ddadd(vanbm, simd.ddmul("va" + str(m), "vb" + str(n))) + ";")
							impl.line()

						impl.line("a += 2*{mr};".format(mr=mr))
						impl.line("b += {nr};".format(nr=nr))
					impl.line("} while (--k);")
					impl.line()

					for m in range(mr // simd.width):
						for n in range(nr):
							impl.line("{ddvec} vc{m}{n} = {ddloaddeinterleave}(&c[{n}*{mr}+{m}*{simd_width}]);".format(
								ddvec=simd.ddvec, m=m, n=n, mr=mr, simd_width=simd.width, ddloaddeinterleave=simd.ddloaddeinterleave))
					impl.line()

					for m in range(mr // simd.width):
						for n in range(nr):
							impl.line("vc{m}{n} = {ddadd}(vc{m}{n}, va{m}b{n});".format(m=m, n=n, ddadd=simd._ddadd))
					impl.line()

					for m in range(mr // simd.width):
						for n in range(nr):
							impl.line("{ddinterleavestore}(&c[{n}*{mr}+{m}*{simd_width}], vc{m}{n});".format(
								m=m, n=n, mr=mr, simd_width=simd.width, ddinterleavestore=simd.ddinterleavestore))
				impl.line("}")
				impl.line()

	with CodeWriter() as header:
		header.line("""\
#pragma once

#include <fpplus.h>

#ifdef __cplusplus
extern "C" {{
#endif

#define DDGEMM_MR_MIN {mr_min}
#define DDGEMM_MR_MAX {mr_max}
#define DDGEMM_MR_STEP {mr_step}
#define DDGEMM_NR_MIN {nr_min}
#define DDGEMM_NR_MAX {nr_max}

typedef void (*ddgemm_function)(size_t, const double*, const doubledouble*, doubledouble*);

""".format(mr_min=options.mr_min, mr_max=options.mr_max, mr_step=simd.width, nr_min=options.nr_min, nr_max=options.nr_max))
		for mr in range(options.mr_min, options.mr_max + 1, simd.width):
			for nr in range(options.nr_min, options.nr_max + 1):
				header.line("void ddgemm{mr}x{nr}(size_t k, const double a[], const doubledouble b[], doubledouble c[]);"
					.format(mr=mr, nr=nr))

		header.line()
		header.line()

		header.line("static inline ddgemm_function select_ddgemm_kernel(size_t mr, size_t nr) {")
		with CodeBlock():
			header.line("switch (mr) {")
			with CodeBlock():
				for mr in range(options.mr_min, options.mr_max + 1, simd.width):
					header.line("case {mr}:".format(mr=mr))
					with CodeBlock():
						header.line("switch (nr) {")
						with CodeBlock():
							for nr in range(options.nr_min, options.nr_max + 1):
								header.line("case {nr}:".format(nr=nr))
								header.indent_line("return ddgemm{mr}x{nr};".format(mr=mr, nr=nr))
							header.line("default:")
							header.indent_line("return NULL;")
						header.line("}")
				header.line("default:")
				header.indent_line("return NULL;")
			header.line("}")
		header.line("}")

		header.line("""

#ifdef __cplusplus
} /* extern "C" */
#endif
""")


	with CodeWriter() as unittest:
		unittest.line("""\
#include <cstddef>
#include <cstdlib>

#include <gtest/gtest.h>

#include <ddgemm/ddgemm.h>

#include "ddgemm-tester.h"

""")
		for mr in range(options.mr_min, options.mr_max + 1, simd.width):
			for nr in range(options.nr_min, options.nr_max + 1):
				unittest.line("""\
TEST(ddgemm, ukernel{mr}x{nr}) {{
	DDGEMMTester<{mr}, {nr}, {simd_width}, ddgemm{mr}x{nr}>().test();
}}
""".format(mr=mr, nr=nr, simd_width=simd.width))

		unittest.line("""\
int main(int argc, char* argv[]) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
""")

	with open(options.implementation, "w") as impl_file:
		impl_file.write(str(impl))

	with open(options.header, "w") as header_file:
		header_file.write(str(header))

	with open(options.unittest, "w") as unittest_file:
		unittest_file.write(str(unittest))


if __name__ == "__main__":
	sys.exit(main())

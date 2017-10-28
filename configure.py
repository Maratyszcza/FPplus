#!/usr/bin/env python

from __future__ import print_function

import os
import sys
import glob
import argparse
import ninja_syntax


def get_program_info(program, arguments, use_stdout=True):
    from subprocess import PIPE, Popen

    if not isinstance(arguments, list):
        arguments = [str(arguments)]
    process = Popen([program] + arguments, stdout=PIPE, stderr=PIPE, bufsize=1)
    outdata, errdata = process.communicate()
    if use_stdout:
        return outdata
    else:
        return errdata


def detect_compiler(program):
    banner = get_program_info(program, "--version")
    if banner:
        identification = banner.splitlines()[0].decode("utf-8")
        import re
        intel_match = re.match(r"(icc|icpc) \(ICC\) (\d+(:?\.\d+)+)", identification)
        if intel_match:
            return "Intel", intel_match.group(2)
        gnu_match = re.match(r"(gcc|g\+\+)(?:\-\d+)? \(.*\) (\d+(:?\.\d+)+)", identification)
        if gnu_match:
            return "GNU", gnu_match.group(2)
        clang_match = re.match(r".*\bclang version (\d+(:?\.\d+)+)", identification)
        if clang_match:
            return "Clang", clang_match.group(1)
        apple_match = re.match(r"Apple LLVM version (\d+(:?\.\d+)+)", identification)
        if apple_match:
            return "Apple", apple_match.group(1)


class Configuration:
    def __init__(self, options, root_dir):
        self.build = Configuration.detect_build_platform()
        print("Build platform: {build}".format(build=self.build))
        self.host = Configuration.detect_host_platform(self.build, options.uarch)
        print("Host platform: {host}".format(host=self.host))

        self.writer = ninja_syntax.Writer(open(os.path.join(root_dir, "build.ninja"), "w"))
        self.root_dir = root_dir
        self.source_dir = os.path.join(root_dir, "src")
        self.build_dir = os.path.join(root_dir, "build")
        self.include_dirs = [os.path.join(root_dir, "include")]
        self.binaries_dir = os.path.join(root_dir, "bin")
        self.mflags = []
        self.cflags = []
        self.cxxflags = []
        self.ldflags = []
        self.lddirs = []
        self.ldlibs = []
        self.macros = []
        self.object_ext = ".o"

        cc, cxx = self.setup_compilers(options.cc, options.cxx)
        self.setup_compiler_options(options.uarch)

        if options.fpaddre:
            self.macros.append("FPPLUS_EMULATE_FPADDRE")
        if options.quad:
            self.macros.append("FPPLUS_HAVE_FLOAT128")
        self.macros.append("FPPLUS_UARCH_" + options.uarch.upper())


        # Variables
        self.writer.variable("cc", cc)
        self.writer.variable("cxx", cxx)
        self.writer.variable("mflags", " ".join(self.mflags))
        self.writer.variable("cflags", " ".join(self.cflags))
        self.writer.variable("cxxflags", " ".join(self.cxxflags))
        self.writer.variable("ldflags", " ".join(self.ldflags))
        self.writer.variable("macros", " ".join("-D" + macro for macro in self.macros))

        # Rules
        self.writer.rule("cc", "$cc $mflags $cflags $includes -o $out -c $in -MMD -MF $out.d",
            deps="gcc", depfile="$out.d",
            description="CC $descpath")
        self.writer.rule("cxx", "$cxx $mflags $cxxflags $includes -o $out -c $in -MMD -MF $out.d",
            deps="gcc", depfile="$out.d",
            description="CXX $descpath")
        self.writer.rule("ccld", "$cc $mflags $ldflags $lddirs -o $out $in $ldlibs",
            description="CCLD $descpath")
        self.writer.rule("cxxld", "$cxx $mflags $ldflags $lddirs -o $out $in $ldlibs",
            description="CXXLD $descpath")
        self.writer.rule("dot", "python $in --unroll-min $unroll_min --unroll-max $unroll_max --simd $simd --implementation $implementation --header $header --unittest $unittest",
            description="GEN $descpath") 
        self.writer.rule("gemm", "python $in --mr-min $mr_min --mr-max $mr_max --nr-min $nr_min --nr-max $nr_max --simd $simd --implementation $implementation --header $header --unittest $unittest",
            description="GEN $descpath") 


    @staticmethod
    def detect_build_platform():
        import sys
        if sys.platform.startswith("linux"):
            return "x86_64-linux-gnu"
        elif sys.platform == "darwin":
            return "x86_64-osx"
        else:
            print("Error: failed to detect build platform: sys.platform = {platform}"
                .format(platform=sys.platform), file=sys.stdout)
            sys.exit(1)


    @staticmethod
    def detect_host_platform(build, uarch):
        if uarch == "knc":
            return "k1om-linux-gnu"
        else:
            return build


    def setup_compilers(self, cc, cxx):
        if cc is None and cxx is None:
            cc, cxx = {
                "x86_64-linux-gnu": ("gcc", "g++"),
                "x86_64-osx": ("clang", "clang++"),
                "k1om-linux-gnu": ("icc", "icpc"),
            }[self.host]
        elif cc is None:
            import re
            cc = cxx
            cc = re.sub(r"\bclang\+\+\b", "clang", cc)
            cc = re.sub(r"\bicpc\b", "icc", cc)
            cc = re.sub(r"\bg\+\+\b", "gcc", cc)
            if cc == cxx:
                print("Error: failed to auto-detect C compiler from C++ compiler: use --with-cc to specify C compiler explicitly", file=sys.stderr)
                sys.exit(1)
        elif cxx is None:
            import re
            cxx = cc
            cxx = re.sub(r"\bclang\b", "clang++", cxx)
            cxx = re.sub(r"\bicc\b", "icpc", cxx)
            cxx = re.sub(r"\bgcc\b", "g++", cxx)
            if cxx == cc:
                print("Error: failed to auto-detect C++ compiler from C compiler: use --with-cxx to specify C++ compiler explicitly", file=sys.stderr)
                sys.exit(1)
        cc_name, cc_version = detect_compiler(cc)
        cxx_name, cxx_version = detect_compiler(cxx)
        print("C compiler: {name} {version}".format(name=cc_name, version=cc_version))
        print("C++ compiler: {name} {version}".format(name=cxx_name, version=cxx_version))
        if (cc_name, cc_version) != (cxx_name, cxx_version):
            print("Error: C and C++ toolchain mismatch", file=sys.stderr)
            sys.exit(1)
        self.compiler_id = cc_name
        return cc, cxx


    def setup_compiler_options(self, uarch):
        if uarch == "knc":
            self.mflags = ["-mmic"]
        else:
            self.mflags = ["-m64"]
            if self.compiler_id == "Intel":
                isaflag = {
                    "haswell": "-xCORE-AVX2",
                    "broadwell": "-xCORE-AVX2",
                    "skylake": "-xCORE-AVX2",
                }[uarch]
            else:
                isaflag = {
                    "haswell": "-march=core-avx2",
                    "broadwell": "-march=broadwell",
                    "skylake": "-mavx2 -mfma",
                    "bulldozer": "-march=bdver1",
                    "piledriver": "-march=bdver2",
                    "steamroller": "-march=bdver3",
                }[uarch]
            self.cflags.append(isaflag)
            self.cxxflags.append(isaflag)

        self.cflags += ["-std=gnu99", "-g", "-O3", "-Wall", "-Wextra", "-Wno-unused-parameter"]
        self.cxxflags += ["-std=gnu++11", "-g", "-O3", "-Wall", "-Wextra", "-Wno-unused-parameter", "-Wno-missing-field-initializers"]
        if self.compiler_id == "Intel":
            self.cflags += ["-fp-model", "precise", "-no-fma"]
            self.cxxflags += ["-fp-model", "precise", "-no-fma"]
        else:
            self.cflags.append("-ffp-contract=off")
            self.cxxflags.append("-ffp-contract=off")

        if self.host in ["x86_64-linux-gnu", "k1om-linux-gnu"]:
            self.cxxflags.append("-pthread")
            self.ldflags.append("-pthread")


    def cc(self, source_file, object_file=None):
        if not os.path.isabs(source_file):
            source_file = os.path.join(self.source_dir, source_file)
        if object_file is None:
            object_file = os.path.join(self.build_dir, os.path.relpath(source_file, self.source_dir)) + self.object_ext
        variables = {
            "descpath": os.path.relpath(source_file, self.source_dir)
        }
        if self.include_dirs:
            variables["includes"] = " ".join(map(lambda include_dir: "-I" + include_dir, self.include_dirs))
        if self.macros:
            variables["cflags"] = "$cflags " + " ".join(map(lambda macro: "-D" + macro, self.macros))
        self.writer.build(object_file, "cc", source_file, variables=variables)
        return object_file


    def cxx(self, source_file, object_file=None):
        if not os.path.isabs(source_file):
            source_file = os.path.join(self.source_dir, source_file)
        if object_file is None:
            object_file = os.path.join(self.build_dir, os.path.relpath(source_file, self.source_dir)) + self.object_ext
        variables = {
            "descpath": os.path.relpath(source_file, self.source_dir)
        }
        if self.include_dirs:
            variables["includes"] = " ".join(map(lambda include_dir: "-I" + include_dir, self.include_dirs))
        if self.macros:
            variables["cxxflags"] = "$cxxflags " + " ".join(map(lambda macro: "-D" + macro, self.macros))
        self.writer.build(object_file, "cxx", source_file, variables=variables)
        return object_file


    def ccld(self, object_files, executable_file, lddirs=[], ldlibs=[]):
        if not os.path.isabs(executable_file):
            executable_file = os.path.join(self.binaries_dir, executable_file)
        variables = {
            "descpath": os.path.relpath(executable_file, self.binaries_dir)
        }
        if self.lddirs or lddirs:
            variables["libdirs"] = " ".join("-L" + lddir for lddir in self.lddirs + lddirs)
        if self.ldlibs or ldlibs:
            variables["ldlibs"] = " ".join("-l" + ldlib for ldlib in self.ldlibs + ldlibs)
        self.writer.build(executable_file, "ccld", object_files, variables=variables)
        return executable_file


    def cxxld(self, object_files, executable_file, lddirs=[], ldlibs=[]):
        if not os.path.isabs(executable_file):
            executable_file = os.path.join(self.binaries_dir, executable_file)
        variables = {
            "descpath": os.path.relpath(executable_file, self.binaries_dir)
        }
        if self.lddirs or lddirs:
            variables["libdirs"] = " ".join("-L" + lddir for lddir in self.lddirs + lddirs)
        if self.ldlibs or ldlibs:
            variables["ldlibs"] = " ".join("-l" + ldlib for ldlib in self.ldlibs + ldlibs)
        self.writer.build(executable_file, "cxxld", object_files, variables=variables)
        return executable_file


    def dot(self, unroll_min, unroll_max, simd):
        implementation_file = os.path.join(self.source_dir, "dot", "dot-{simd}.c".format(simd=simd))
        header_file = os.path.join(self.source_dir, "dot", "dot.h")
        unittest_file = os.path.join(self.root_dir, "test", "dot.cpp")
        script_file = os.path.join(self.source_dir, "dot", "dot.py")
        variables = {
            "descpath": os.path.relpath(implementation_file, self.source_dir),
            "unroll_min": str(unroll_min),
            "unroll_max": str(unroll_max),
            "simd": simd,
            "implementation": implementation_file,
            "header": header_file,
            "unittest": unittest_file
        }
        self.writer.build(
            [implementation_file, header_file, unittest_file],
            "dot", script_file, variables=variables)
        return implementation_file, header_file, unittest_file


    def gemm(self, mr_min, mr_max, nr_min, nr_max, simd):
        implementation_file = os.path.join(self.source_dir, "ddgemm", "ddgemm-{simd}.c".format(simd=simd))
        header_file = os.path.join(self.source_dir, "ddgemm", "ddgemm.h")
        unittest_file = os.path.join(self.root_dir, "test", "ddgemm.cpp")
        script_file = os.path.join(self.source_dir, "ddgemm", "ddgemm.py")
        variables = {
            "descpath": simd,
            "mr_min": str(mr_min),
            "mr_max": str(mr_max),
            "nr_min": str(nr_min),
            "nr_max": str(nr_max),
            "simd": simd,
            "implementation": implementation_file,
            "header": header_file,
            "unittest": unittest_file
        }
        self.writer.build(
            [implementation_file, header_file, unittest_file],
            "gemm", script_file, variables=variables)
        return implementation_file, header_file, unittest_file


parser = argparse.ArgumentParser(description="FP+ configuration script")
parser.add_argument("--enable-fpaddre", dest="fpaddre", action="store_true", default=False,
    help="Emulate FPADDRE instruction")
parser.add_argument("--enable-quad", dest="quad", action="store_true", default=False,
    help="Enable quad-precision benchmark (requires gcc or icc)")
parser.add_argument("--uarch", dest="uarch", required=True,
    choices=("haswell", "broadwell", "skylake", "bulldozer", "piledriver", "steamroller", "knc"),
    help="Target micro-architecture")
parser.add_argument("--with-cc", dest="cc", default=os.getenv("CC"),
    help="C compiler to use")
parser.add_argument("--with-cxx", dest="cxx", default=os.getenv("CXX"),
    help="C++ compiler to use")
parser.add_argument("--with-gmp", dest="gmp",
    help="Path to GNU MP prefix dir")
parser.add_argument("--with-mpfr", dest="mpfr",
    help="Path to MPFR prefix dir")


def main():
    options = parser.parse_args()
    root_dir = os.path.dirname(os.path.abspath(__file__))
    config = Configuration(options, root_dir)

    # Build gtest
    gtest_dir = os.path.join(root_dir, "third-party", "googletest")
    config.source_dir = os.path.join(gtest_dir, "src")
    config.build_dir = os.path.join(root_dir, "build", "gtest")
    config.include_dirs = [os.path.join(gtest_dir, "include"), gtest_dir]
    gtest_object = config.cxx("gtest-all.cc")

    # Setup
    config.source_dir = os.path.join(root_dir, "src")
    config.build_dir = os.path.join(root_dir, "build")
    config.include_dirs = [
        os.path.join(root_dir, "include"),
        os.path.join(root_dir, "src"),
    ]
    config.ldlibs = ["m"]
    if sys.platform.startswith("linux"):
        config.ldlibs.append("rt")

    # Build benchmarks
    simd = "avx"
    simd_width = 4
    if options.uarch == "knc":
        simd = "mic"
        simd_width = 8

    utils_object = config.cc("utils.c")

    dot_source, dot_header, dot_test_source = config.dot(1, 8, simd)
    dot_object = config.cc(dot_source)
    dot_sources = ["dot.c", "dot/dot.c"]
    config.ccld([
        config.cc("dot/benchmark.c"),
        config.cc("dot/options.c"),
        dot_object, utils_object], "dot-bench")

    gemm_source, gemm_header, gemm_test_source = config.gemm(simd_width, simd_width * 3, 1, 8, simd)
    gemm_object = config.cc(gemm_source)
    config.ccld([
        config.cc("ddgemm/benchmark.c"),
        config.cc("ddgemm/options.c"),
        gemm_object, utils_object], "ddgemm-bench")

    ubench_objects = [
        config.cc("low-level/benchmark.c"),
        config.cc("low-level/options.c"),
        config.cc("low-level/doubledouble.c"),
        config.cc("low-level/polevl.c")]
    if options.quad:
        ubench_objects.append(config.cc("low-level/quad.c"))
    config.ccld(ubench_objects, "ubench")

    # Build tests
    config.source_dir = os.path.join(root_dir, "test")
    config.build_dir = os.path.join(root_dir, "build", "test")
    config.include_dirs = [
        os.path.join(root_dir, "include"),
        os.path.join(root_dir, "src"),
        os.path.join(gtest_dir, "include")
    ]

    if config.host != config.build and (options.mpfr is None or options.gmp is None):
        print("Warning: cannot cross-compile tests without explicitly specified paths to MPFR and GMP, see --with-mpfr and --with-gmp options")
    elif not options.fpaddre:
        test_ldlibs, test_ldobjs = [], []
        if options.mpfr:
            test_ldobjs.append(os.path.join(options.mpfr, "lib", "libmpfr.a"))
            config.include_dirs.append(os.path.join(options.mpfr, "include"))
        else:
            test_ldlibs.append("mpfr")
        if options.gmp:
            test_ldobjs.append(os.path.join(options.gmp, "lib", "libgmp.a"))
            config.include_dirs.append(os.path.join(options.gmp, "include"))
        else:
            test_ldlibs.append("gmp")
        config.cxxld([config.cxx("error-free-transform.cpp"), gtest_object] + test_ldobjs,
            "eft-test", ldlibs=test_ldlibs)
        config.cxxld([config.cxx("double-double.cpp"), gtest_object] + test_ldobjs,
            "dd-test", ldlibs=test_ldlibs)
        config.cxxld([config.cxx("dot.cpp"), dot_object, gtest_object] + test_ldobjs,
            "dot-test", ldlibs=test_ldlibs)
        config.cxxld([config.cxx("ddgemm.cpp"), gemm_object, gtest_object] + test_ldobjs,
            "ddgemm-test", ldlibs=test_ldlibs)


if __name__ == "__main__":
    sys.exit(main())

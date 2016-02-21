# FPplus
Scientific library for high-precision computations and research

FPplus was originally developed for a research project on instructions to accelerate high-precision computations, but it is also useful as a general-purpose library. FPplus features:

- Header-only library for error-free transforms and double-double computations
  - Implements error-free addition, multiplication, and fused multiply-add
  - Implements double-double addition and multiplication in multiple variants
- Compatible with C99, C++, OpenCL, and CUDA
- Special versions of error-free transforms in SIMD intrinsics:
  - x86 SIMD (128-bit and 256-bit AVX + FMA, 512-bit wide MIC and AVX-512)
  - IBM VSX (POWER 7 and POWER 8) and QPX (Blue Gene/Q)
  - ARMv8 SIMD
- Extensive documentation with references to scientific literature
- Testsuite based on [MPFR](http://www.mpfr.org/) and [Google Test](https://github.com/google/googletest)
- Examples and code-generators for high-precision algorithms:
  - Polynomial evaluation with compensated Horner scheme
  - Compensated dot product algorithm
  - Inner kernel of matrix multiplication (GEMM) operation in double-double precision

## Requirements

##### CPU targets:
- gcc-compatible compiler (tested on gcc, clang and icc)
- Hardware FMA support
- Precise floating-point semantics
  - No `-ffast-math` option when compiling with `gcc` or `clang`
  - `-fp-model precise` when compiling with `icc`

##### OpenCL targets:
- `cl_khr_fp64`, `cl_amd_fp64`, or `cl_APPLE_fp64_basic_ops` extension
- Hardware FMA support (`FP_FAST_FMA` must be defined by OpenCL compiler)
- Precise floating-point semantics
  - No `-cl-fast-relaxed-math` option

##### CUDA targets:
- Compute capability 2.0 or higher

## Using FPplus

```c
#include <fpplus.h>
```

Acknowledgements
----------------

[![HPC Garage logo](https://github.com/Maratyszcza/PeachPy/blob/master/logo/hpcgarage.png)](http://hpcgarage.org)
[![Georgia Tech College of Computing logo](https://github.com/Maratyszcza/PeachPy/blob/master/logo/college-of-computing.gif)](http://www.cse.gatech.edu/)

The library was developed by [Marat Dukhan](http://www.maratdukhan.com) as a research project at [Richard Vuduc](http://vuduc.org)'s HPC Garage lab in the Georgia Institute of Technology, College of Computing, School of Computational Science and Engineering. FPplus is based on algorithms in [Handbook of Floating-Point Arithmetics](https://www.springer.com/us/book/9780817647049), [David Bailey](http://davidhbailey.com/)'s QD library, the works of [Jonathan Shewchuk](http://www.cs.berkeley.edu/~jrs/papers/robustr.pdf), [Theodorus Dekker](https://dl.acm.org/citation.cfm?id=2717032), [Donald Knuth](https://dl.acm.org/citation.cfm?id=270146), and [Sylvie Boldo and Jean-Michel Muller](http://www.ens-lyon.fr/LIP/Pub/Rapports/RR/RR2004/RR2004-41.pdf). We thank [Jason Riedy](http://www.cc.gatech.edu/~jriedy) for his feedback and support.

This material is based upon work supported by the U.S. National Science Foundation (NSF) Award Number 1339745 and the U.S. Dept. of Energy (DOE), Office of Science, Advanced Scientific Computing Research under award DE-FC02-10ER26006/DE-SC0004915. Any opinions, findings and conclusions or recommendations expressed in this material are those of the authors and do not necessarily reflect those of NSF or DOE.

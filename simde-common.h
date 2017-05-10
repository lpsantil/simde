/* Copyright (c) 2017 Evan Nemerson <evan@nemerson.com>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#if !defined(SIMDE_COMMON_H)
#define SIMDE_COMMON_H

#include "hedley.h"

#define SIMDE__CONCAT(a,b) a##b
#define SIMDE__XCONCAT(a,b) SIMDE__CONCAT(a,b)

#include "simde-arch.h"

#if defined(__GNUC__)
#  define SIMDE__ALIGN(alignment) __attribute__((aligned(alignment)))
#elif defined(_MSC_VER)
#  define SIMDE__ALIGN(alignment) __declspec(align(alignment))
#else
#  define SIMDE__ALIGN(alignment)
#endif

#if HEDLEY_GCC_HAS_ATTRIBUTE(vector_size,4,6,0)
#  define SIMDE__ENABLE_GCC_VEC_EXT
#endif

#if !defined(SIMDE_ENABLE_OPENMP) && ((defined(_OPENMP) && (_OPENMP >= 201307L)) || (defined(_OPENMP_SIMD) && (_OPENMP_SIMD >= 201307L)))
#  define SIMDE_ENABLE_OPENMP
#endif

#if !defined(SIMDE_ENABLE_CILKPLUS) && defined(__cilk)
#  define SIMDE_ENABLE_CILKPLUS
#endif

#if defined(SIMDE_ENABLE_OPENMP)
#  define SIMDE__VECTORIZE _Pragma("omp simd")
#  define SIMDE__VECTORIZE_SAFELEN(l) HEDLEY_PRAGMA(omp simd safelen(l))
#  define SIMDE__VECTORIZE_REDUCTION(r) HEDLEY_PRAGMA(omp simd reduction(r))
#  define SIMDE__VECTORIZE_ALIGNED(a) HEDLEY_PRAGMA(omp simd aligned(a))
#elif defined(SIMDE_ENABLE_CILKPLUS)
#  define SIMDE__VECTORIZE _Pragma("simd")
#  define SIMDE__VECTORIZE_SAFELEN(l) HEDLEY_PRAGMA(simd vectorlength(l))
#  define SIMDE__VECTORIZE_REDUCTION(r) HEDLEY_PRAGMA(simd reduction(r))
#  define SIMDE__VECTORIZE_ALIGNED(a) HEDLEY_PRAGMA(simd aligned(a))
#elif defined(__INTEL_COMPILER)
#  define SIMDE__VECTORIZE _Pragma("simd")
#  define SIMDE__VECTORIZE_SAFELEN(l) HEDLEY_PRAGMA(simd vectorlength(l))
#  define SIMDE__VECTORIZE_REDUCTION(r) HEDLEY_PRAGMA(simd reduction(r))
#  define SIMDE__VECTORIZE_ALIGNED(a)
#elif defined(__clang__)
#  define SIMDE__VECTORIZE _Pragma("clang loop vectorize(enable)")
#  define SIMDE__VECTORIZE_SAFELEN(l) HEDLEY_PRAGMA(clang loop vectorize_width(l))
#  define SIMDE__VECTORIZE_REDUCTION(r) SIMDE__VECTORIZE
#  define SIMDE__VECTORIZE_ALIGNED(a)
#elif HEDLEY_GCC_VERSION_CHECK(4,9,0)
#  define SIMDE__VECTORIZE _Pragma("GCC ivdep")
#  define SIMDE__VECTORIZE_SAFELEN(l) SIMDE__VECTORIZE
#  define SIMDE__VECTORIZE_REDUCTION(r) SIMDE__VECTORIZE
#  define SIMDE__VECTORIZE_ALIGNED(a)
#else
#  define SIMDE__VECTORIZE
#  define SIMDE__VECTORIZE_SAFELEN(l)
#  define SIMDE__VECTORIZE_REDUCTION(r)
#  define SIMDE__VECTORIZE_ALIGNED(a)
#endif

#if HEDLEY_GCC_HAS_ATTRIBUTE(unused,3,1,0)
#  define SIMDE__UNUSED __attribute__((__unused__))
#else
#  define SIMDE__UNUSED
#endif

#if HEDLEY_GCC_HAS_ATTRIBUTE(artificial,4,3,0)
#  define SIMDE__ARTIFICIAL __attribute__((__artificial__))
#else
#  define SIMDE__ARTIFICIAL
#endif

/* Intended for checking coverage, you should never use this in
   production. */
#if defined(SIMDE_NO_INLINE)
#  define SIMDE__FUNCTION_ATTRIBUTES HEDLEY_NEVER_INLINE SIMDE__UNUSED static
#else
#  define SIMDE__FUNCTION_ATTRIBUTES HEDLEY_INLINE SIMDE__ARTIFICIAL static
#endif

#if defined(_MSC_VER)
#  define SIMDE__BEGIN_DECLS HEDLEY_DIAGNOSTIC_PUSH __pragma(warning(disable:4996 4204)) HEDLEY_BEGIN_C_DECLS
#  define SIMDE__END_DECLS HEDLEY_DIAGNOSTIC_POP HEDLEY_END_C_DECLS
#else
#  define SIMDE__BEGIN_DECLS HEDLEY_BEGIN_C_DECLS
#  define SIMDE__END_DECLS HEDLEY_END_C_DECLS
#endif

#if defined(__SIZEOF_INT128__)
#  define SIMDE__HAVE_INT128
typedef __int128 simde_int128;
typedef unsigned __int128 simde_uint128;
#endif

/* TODO: we should at least make an attempt to detect the correct
   types for simde_float32/float64 instead of just assuming float and
   double. */

#if !defined(SIMDE_FLOAT32_TYPE)
#  define SIMDE_FLOAT32_TYPE float
#endif
typedef SIMDE_FLOAT32_TYPE simde_float32;
HEDLEY_STATIC_ASSERT(sizeof(simde_float32) == 4, "Unable to find 32-bit floating-point type.");

#if !defined(SIMDE_FLOAT64_TYPE)
#  define SIMDE_FLOAT64_TYPE double
#endif
typedef SIMDE_FLOAT64_TYPE simde_float64;
HEDLEY_STATIC_ASSERT(sizeof(simde_float64) == 8, "Unable to find 64-bit floating-point type.");

/* This will probably move into Hedley at some point, but I'd like to
   more thoroughly check for other compilers which define __GNUC__
   first. */
#if defined(SIMDE__REALLY_GCC)
#  undef SIMDE__REALLY_GCC
#endif
#if !defined(__GNUC__) || \
  defined(__clang__) || \
  defined(__INTEL_COMPILER)
#define SIMDE__REALLY_GCC 0
#else
#define SIMDE__REALLY_GCC 1
#endif

/* Sometimes we run into problems with specific versions of compilers
   which make the native versions unusable for us.  Often this is due
   to missing functions, sometimes buggy implementations, etc.  These
   macros are how we check for specific bugs.  As they are fixed we'll
   start only defining them for problematic compiler versions. */

#if !defined(SIMDE_IGNORE_COMPILER_BUGS)
#  if SIMDE__REALLY_GCC
#    if !HEDLEY_GCC_VERSION_CHECK(4,9,0)
#      define SIMDE_BUG_GCC_REV_208793
#    endif
#    if !HEDLEY_GCC_VERSION_CHECK(5,0,0)
#      define SIMDE_BGU_GCC_BAD_MM_SRA_EPI32 /* TODO: find relevant bug or commit */
#    endif
#  endif
#endif

#endif /* !defined(SIMDE_COMMON_H) */

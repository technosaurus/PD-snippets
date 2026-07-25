
#if defined(__AVX512BW__)||defined(__AVX2__)||defined(__SSE2__)
#define USEVMAX
#define VMAXFUNC(func,...) PASTE(VMAXPREFIX,func)(__VA_ARGS__)
#define VMAXFUNC_(func,...) PASTE(PASTE(VMAXPREFIX,func),VMAXBITS)(__VA_ARGS__)

#ifdef __AVX512BW__

#define VMAXPREFIX _mm512_
#define VMAXBITS 512
typedef __m512i mmaxi;
#define VMAXALIGNMASK 63UL
#define VCMPMASK(cmp,typ,...) _mm512_##cmp##_##typ##_mask(__VA_ARGS__)

#elif defined(__AVX2__)

#define VMAXPREFIX _mm256_
#define VMAXBITS 256
typedef __m256i mmaxi;
#define VMAXALIGNMASK 31UL
#define VCMPMASK(cmp,typ,...) _mm256_movemask_##typ(_mm256_##cmp##_##typ(__VA_ARGS__))

#elif defined(__SSE2__)

#define VMAXPREFIX _mm_
#define VMAXBITS 128
typedef __m128i mmaxi;
#define VMAXALIGNMASK 15UL
#define VCMPMASK(cmp,typ,...) _mm_movemask_##typ(_mm_##cmp##_##typ(__VA_ARGS__))

#endif

#endif

#if 0
/* This version uses no architecture specific intrinsics
   All operations are scalable from float to 512bit vectors
   when inlined the operations are blazing fast
   TODO when compiled -Os should drastically reduce size
*/
#ifndef FAST_MATH_H
#define FAST_MATH_H

#include <stddef.h>

/* ==========================================================================
   1. HARDWARE LAYER & SIMD VECTOR DEFINITIONS
   ========================================================================== */
#if defined(_MSC_VER)
    #define FORCE_INLINE __forceinline
    #define ALIGN_64 __declspec(align(64))
#else
    #define FORCE_INLINE inline __attribute__((always_inline))
    #define ALIGN_64 __attribute__((aligned(64)))
#endif

/* Native compiler vector types (GCC/Clang/Apple Silicon/Intel LLVM) */
typedef float  v4sf  __attribute__((vector_size(16)));  /* 4x float  (SSE2/NEON) */
typedef double v2df  __attribute__((vector_size(16)));  /* 2x double (SSE2/NEON) */

typedef float  v8sf  __attribute__((vector_size(32)));  /* 8x float  (AVX2)      */
typedef double v4df  __attribute__((vector_size(32)));  /* 4x double (AVX2)      */

typedef float  v16sf __attribute__((vector_size(64)));  /* 16x float (AVX-512)   */
typedef double v8df  __attribute__((vector_size(64)));  /* 8x double (AVX-512)   */

/* Dual-output structure packages for register-return passing */
typedef struct { float sin_val; float cos_val; }   sincos_f;
typedef struct { double sin_val; double cos_val; } sincos_d;
typedef struct { v4sf sin_val; v4sf cos_val; }     sincos_v4sf;
typedef struct { v2df sin_val; v2df cos_val; }     sincos_v2df;
typedef struct { v8sf sin_val; v8sf cos_val; }     sincos_v8sf;
typedef struct { v4df sin_val; v4df cos_val; }     sincos_v4df;
typedef struct { v16sf sin_val; v16sf cos_val; }   sincos_v16sf;
typedef struct { v8df sin_val; v8df cos_val; }     sincos_v8df;

/* ==========================================================================
   2. POLYNOMIAL DATABASE STRUCTS & X-MACRO REGISTRY
   ========================================================================== */
#define MAX_COEFFS 8

typedef enum { 
    PARITY_GENERAL, 
    PARITY_ODD, 
    PARITY_EVEN 
} PolyParity;

typedef struct {
    ALIGN_64 double coeffs[MAX_COEFFS];
    int degree;
    PolyParity parity;
    int iterations;
} Polynomial;

/* 
   MASTER ENGINE DATABASE
   Format: Name, Degree, Parity, Newton Iterations, Coefficients {c0, c1, ...}
   Values below are standard placeholders; plug your lolremez outputs here!
*/
#define ENGINE_FUNCTIONS \
    X(sin,  2, PARITY_ODD,     0, {0.99999663, -0.16664824, 0.00830627, 0.0, 0.0, 0.0, 0.0, 0.0}) \
    X(cos,  2, PARITY_EVEN,    0, {1.00000000, -0.49999999, 0.04166663, 0.0, 0.0, 0.0, 0.0, 0.0}) \
    X(exp2, 3, PARITY_GENERAL, 0, {1.00000000,  0.69314718, 0.24022650, 0.05550411, 0.0, 0.0, 0.0, 0.0})

typedef enum {
#define X(name, deg, parity, iters, coeffs) ENUM_##name,
    ENGINE_FUNCTIONS
#undef X
    NUM_ENGINE_FUNCTIONS
} MathFunctionEnum;

/* Static private scope ensures the compiler converts this array to dead code post-inlining */
static const Polynomial g_PolyRegistry[NUM_ENGINE_FUNCTIONS] = {
#define X(name, deg, parity, iters, coeffs) [ENUM_##name] = { coeffs, deg, parity, iters },
    ENGINE_FUNCTIONS
#undef X
};

/* Type Matrix for Dual-Loop Code Generation Suffixes */
#define MATH_TYPE_MATRIX \
    X(f,     float,  (float))   \
    X(d,     double, (double))  \
    X(v4sf,  v4sf,   (v4sf){})  \
    X(v2df,  v2df,   (v2df){})  \
    X(v8sf,  v8sf,   (v8sf){})  \
    X(v4df,  v4df,   (v4df){})  \
    X(v16sf, v16sf,  (v16sf){}) \
    X(v8df,  v8df,   (v8df){})

/* ==========================================================================
   3. PIPELINE TEMPLATE AND CODE GENERATION
   ========================================================================== */

/* Hardcoded, compiler-unrolled Newton steps (e.g. standard rcp/div iterations) */
#define APPLY_NEWTON_STEPS(iters, res, x, splat) \
    if (iters > 0) { res = res * (splat 2.0 - x * res); } \
    if (iters > 1) { res = res * (splat 2.0 - x * res); } \
    if (iters > 2) { res = res * (splat 2.0 - x * res); }

/* Unified mathematical blueprint for standard functions */
#define GENERATE_MATH_PIPELINE(func_name, type_suffix, T, splat, deg, parity, iters) \
    static FORCE_INLINE T math_##func_name##_##type_suffix(T x, Polynomial p) {       \
        typeof(x) res;                                                               \
        if (parity == PARITY_ODD) {                                                  \
            typeof(x) x2 = x * x;                                                    \
            res = splat + p.coeffs[deg];                                             \
            for (int d = deg - 1; d >= 0; d--) {                                     \
                res = res * x2 + (splat + p.coeffs[d]);                              \
            }                                                                        \
            res = res * x;                                                           \
        } else if (parity == PARITY_EVEN) {                                          \
            typeof(x) x2 = x * x;                                                    \
            res = splat + p.coeffs[deg];                                             \
            for (int d = deg - 1; d >= 0; d--) {                                     \
                res = res * x2 + (splat + p.coeffs[d]);                              \
            }                                                                        \
        } else {                                                                     \
            res = splat + p.coeffs[deg];                                             \
            for (int d = deg - 1; d >= 0; d--) {                                     \
                res = res * x + (splat + p.coeffs[d]);                               \
            }                                                                        \
        }                                                                            \
        APPLY_NEWTON_STEPS(iters, res, x, splat)                                     \
        return res;                                                                  \
    }

/* Unified mathematical blueprint for joint sincos evaluation */
#define GENERATE_SINCOS_PIPELINE(type_suffix, T, splat)                       \
    static FORCE_INLINE sincos_##type_suffix math_sincos_##type_suffix(T x) { \
        Polynomial p_sin = g_PolyRegistry[ENUM_sin];                          \
        Polynomial p_cos = g_PolyRegistry[ENUM_cos];                          \
        typeof(x) x2 = x * x;                                                 \
        /* Sine execution block */                                            \
        typeof(x) s_res = splat + p_sin.coeffs[p_sin.degree];                 \
        for (int d = p_sin.degree - 1; d >= 0; d--) {                         \
            s_res = s_res * x2 + (splat + p_sin.coeffs[d]);                   \
        }                                                                     \
        s_res = s_res * x;                                                    \
        /* Cosine execution block */                                          \
        typeof(x) c_res = splat + p_cos.coeffs[p_cos.degree];                 \
        for (int d = p_cos.degree - 1; d >= 0; d--) {                         \
            c_res = c_res * x2 + (splat + p_cos.coeffs[d]);                   \
        }                                                                     \
        sincos_##type_suffix out = { s_res, c_res };                          \
        return out;                                                           \
    }

/* 
   DUAL-LOOP MACRO EXPLOSION 
   Stamps out explicitly written math functions for every database entry + type pair 
*/
#define X(name, deg, parity, iters, coeffs) \
    #define Y(suffix, T, splat) GENERATE_MATH_PIPELINE(name, suffix, T, splat, deg, parity, iters) \
    MATH_TYPE_MATRIX \
    #undef Y
    ENGINE_FUNCTIONS
#undef X

/* Stamps out specialized sincos variants across the entire type matrix */
#define X(suffix, T, splat) GENERATE_SINCOS_PIPELINE(suffix, T, splat)
    MATH_TYPE_MATRIX
#undef X

/* ==========================================================================
   4. COMPILE-TIME DISPATCH GENERIC INTERFACES
   ========================================================================== */

/* Pre-flattened lines to cleanly maps types to generated static functions */
#define GENERIC_MAP_sin(x, p) _Generic((x), float: math_sin_f, double: math_sin_d, v4sf: math_sin_v4sf, v2df: math_sin_v2df, v8sf: math_sin_v8sf, v4df: math_sin_v4df, v16sf: math_sin_v16sf, v8df: math_sin_v8df)(x, p)
#define GENERIC_MAP_cos(x, p) _Generic((x), float: math_cos_f, double: math_cos_d, v4sf: math_cos_v4sf, v2df: math_cos_v2df, v8sf: math_cos_v8sf, v4df: math_cos_v4df, v16sf: math_cos_v16sf, v8df: math_cos_v8df)(x, p)
#define GENERIC_MAP_exp2(x, p) _Generic((x), float: math_exp2_f, double: math_exp2_d, v4sf: math_exp2_v4sf, v2df: math_exp2_v2df, v8sf: math_exp2_v8sf, v4df: math_exp2_v4df, v16sf: math_exp2_v16sf, v8df: math_exp2_v8df)(x, p)

/* Unified top-level macro interface for single inputs */
#define math(func, x) GENERIC_MAP_##func(x, g_PolyRegistry[ENUM_##func])

/* Unified top-level macro interface for sincos */
#define math_sincos(x) _Generic((x), \
    float:  math_sincos_f,           \
    double: math_sincos_d,           \
    v4sf:   math_sincos_v4sf,        \
    v2df:   math_sincos_v2df,        \
    v8sf:   math_sincos_v8sf,        \
    v4df:   math_sincos_v4df,        \
    v16sf:  math_sincos_v16sf,       \
    v8df:   math_sincos_v8df         \
)(x)

#endif /* FAST_MATH_H */

#endif // 0










#include <stdio.h>

// 1. Define standard Vector types

// 2. Token pasting helpers
#define PASTE_3(a, b, c) a##b##c
#define PASTE_4(a, b, c, d) a##b##c##d

// 3. Selection helpers for standard vs elementwise builtins
#define BUILTIN_STD(name)  __builtin_##name
#define BUILTIN_ELEM(name) PASTE_3(__builtin_, elementwise_, name)
define BUILTIN_CPLX(name)  PASTE_3(__builtin_, c, name)

// 4. The Master Macro (_M) TODO add integer types
#define _M(name, x, ...) _Generic((x), \
    float:              BUILTIN_STD(name), \
    double:             BUILTIN_STD(name), \
    long double:        PASTE_3(__builtin_, name, l), \
    _Float16:           PASTE_3(__builtin_, name, f16), \
    _Float32:           PASTE_3(__builtin_, name, f32), \
    _Float64:           PASTE_3(__builtin_, name, f64), \
    _Float128:          PASTE_3(__builtin_, name, f128), \
    _Decimal32:         PASTE_3(__builtin_, name, d32), \
    _Decimal64:         PASTE_3(__builtin_, name, d64), \
    _Decimal128:        PASTE_3(__builtin_, name, d128), \
    float _Complex:     PASTE_4(__builtin_, c, name, f), \
    double _Complex:    BUILTIN_CPLX(name), \
    long double _Complex: PASTE_4(__builtin_, c, name, l), \
    default:            BUILTIN_ELEM(name), \
)(x, ##__VA_ARGS__)

// 5. Wrap your generic math functions
#define abs(x)   _M(abs,x)
#define acos(x)  _M(acos,x)
//#define add_sat(x,y) _M(add_sat,x,y)
#define asin(x)  _M(asin,x)
#define atan(x)  _M(atan,x)
#define atan2(x,y) _M(atan2,x,y)
//#define bitreverse(x)  _M(bitreverse,x)
#define canonicalize(x) _M(canonicalize,x)
#define ceil(x)   _M(ceil,x)
//#define clzg(x,y) _M(clzg,x,y)
//#define clmul(x,y) _M(clmul,x,y)
#define copysign(x,y) _M(copysign,x,y)
#define cos(x)     _M(cos,x)
#define cosh(x)    _M(cosh,x)
//#define ctzg(x,y) _M(ctzg,x,y)
#define exp(x)     _M(exp,x)
#define exp10(x).  _M(exp10,x)
#define exp2(x)    _M(exp2,x)
#define floor(x)   _M(floor,x)
#define fma(x,y,z) _M(fma,x,y,z)
#define fmod(x,y)  _M(fmod,x,y)
//#define fshl(x,y)  _M(fshl,x,y)
//#define fshr(x,y)  _M(fshr,x,y)
#define ldexp(x,y) _M(ldexp,x,y)
#define log(x)     _M(log,x)
#define log10(x)   _M(log10,x)
#define log2(x)    _M(log2,x)
#define max(x,y)   _M(max,x,y)
#define maximum(x,y) _M(maximum,x,y)
#define maximumnum(x,y) _M(maximumnum,x,y)
#define maxnum(x,y) _M(maxnum,x,y)
#define min(x,y)   _M(min,x,y)
#define minimum(x,y) _M(minimum,x,y)
#define minimumnum(x,y) _M(minimumnum,x,y)
#define minnum(x,y) _M(minnum,x,y)
#define nearbyint(x,y) _M(nearbyint,x)
//#define pdep(x,m)  _M(pdep,x,m)
//#define pext(x,m)  _M(pext,x,m)
//#define popcount(x) _M(popcount,x)
#define pow(x,y)  _M(pow,x,y)
#define rint(x)   _M(rint,x)
#define round(x)  _M(round,x)
#define roundeven(x) _M(roundeven,x)
#define sin(x)    _M(sin,x)
#define sinh(x)   _M(sinh,x)
#define sqrt(x)   _M(sqrt,x)
//#define sub_sat(x,y)  _M(sub_sat,x,y)
#define tan(x)    _M(tan,x)
#define tanh(x)   _M(tanh,x)
#define trunc(x)  _M(trunc,x)




// Reduction Builtins operate on all elements
// __builtin_reduce_max(VT a)
// __builtin_reduce_min(VT a)
// __builtin_reduce_add(VT a)
// __builtin_reduce_mul(VT a)
// __builtin_reduce_and(VT a)
// __builtin_reduce_or(VT a)
// __builtin_reduce_xor(VT a)
// __builtin_reduce_maximum(VT a)
// __builtin_reduce_minimum(VT a)
// __builtin_reduce_assoc_fadd(VT a[, ET s])
// __builtin_reduce_in_order_fadd(VT a, ET s)

### Compiler options that can reduce code size ###

**_-fno-approx-func_** (combined with -ffast-math)

**Example.**
```c
f32x4 sqrtv(f32x4 v4sf){
    return (f32x4){
        __builtin_sqrtf(v4sf[0]),
        __builtin_sqrtf(v4sf[1]),
        __builtin_sqrtf(v4sf[2]),
        __builtin_sqrtf(v4sf[3])
    };
}
```
_with -fno-approx-func and -ffast-math_
```asm
sqrtv:
    vsqrtps xmm0, xmm0
    ret
```
_with -fapprox-func (default)_ with or without -ffast-math
```asm
sqrtv:
    vrsqrtps        xmm1, xmm0
    vmulps  xmm2, xmm0, xmm1
    vbroadcastss    xmm3, dword ptr [rip + .LCPI14_0]
                           # xmm3 = [-5.0E-1,-5.0E-1,-5.0E-1,-5.0E-1]
    vmulps  xmm3, xmm2, xmm3
    vmulps  xmm1, xmm2, xmm1
    vbroadcastss    xmm2, dword ptr [rip + .LCPI14_1]
                           # xmm2 = [-3.0E+0,-3.0E+0,-3.0E+0,-3.0E+0]
    vaddps  xmm1, xmm1, xmm2
    vmulps  xmm1, xmm3, xmm1
    vbroadcastss    xmm2, dword ptr [rip + .LCPI14_2]
                           # xmm2 = [NaN,NaN,NaN,NaN]
    vandps  xmm0, xmm0, xmm2
    vbroadcastss    xmm2, dword ptr [rip + .LCPI14_3]
        # xmm2 = [1.17549435E-38,1.17549435E-38,1.17549435E-38,1.17549435E-38]
    vcmpleps        xmm0, xmm2, xmm0
    vandps  xmm0, xmm0, xmm1
    ret
```
### Compiler + Linker options that can reduce code size ###

`-flto` OR `-ffunction-sections` `-fdata-sections` + `-Wl,--gc-sections`

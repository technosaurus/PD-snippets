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

```sh
# FOR COMPACT UTILITIES (MAXIMUM STORAGE SAVINGS)
clang -Oz -flto=thin -ffunction-sections -fdata-sections \
      -mllvm -enable-gvn-hoist=1 -mllvm -enable-gvn-sink=1 -enable-machine-outliner=always \
      -mllvm -inline-threshold=5 -fno-ident \
      -Wl,--gc-sections -Wl,--icf=all input.c -o output

# FOR COMPUTE ENGINES (MAXIMUM NATIVE SPEED EXECUTIONS)
clang -Ofast -flto=thin -fstrict-aliasing -fomit-frame-pointer \
      -fno-semantic-interposition input.c -o output
```

```sh
#compile llvm ir to tiny binary
clang -x ir - \
    -Oz -flto=thin -fno-ident \
    -fassociative-math -fno-signed-zeros -freciprocal-math \
    -ffinite-math-only -fno-trapping-math -ffp-contract=fast \
    -ffunction-sections -fdata-sections \
    -mllvm -enable-machine-outliner=always \
    -mllvm -enable-gvn-hoist=1 -mllvm -enable-gvn-sink=1 \
    -mllvm -inline-threshold=5 \
    -march=native -s \
    -Wl,--gc-sections -Wl,--icf=all \
    -o "$OUT_FILE"

# Append this to the tail of your build environment to overwrite upstream Makefiles
CFLAGS="-c -emit-llvm \
-march=x86-64 -mno-sse3 -mno-ssse3 -mno-sse4.1 -mno-sse4.2 -mno-avx -mno-avx2 -mno-avx512f \
-Oz -finline-small-functions -mllvm -inline-threshold=25 \
-fvectorize -fslp-vectorize -fno-unroll-loops -fno-vectorize-loops-partially -mllvm -force-vector-interleave=1 \
-fno-exceptions -fno-rtti -fno-asynchronous-unwind-tables \
-fassociative-math -fno-signed-zeros -freciprocal-math -ffinite-math-only -fno-trapping-math -ffp-contract=fast \
-ffunction-sections -fdata-sections -fno-ident -Xclang -opaque-pointers"


#CFLAGS to enable compatible bitcode
# --- 1. CORE BITCODE EMISSION PASS ---
-c -emit-llvm \

# --- 2. THE UPSTREAM OVERRIDE BLOCK (Neutralizes problematic flags) ---
-march=x86-64 -mno-sse3 -mno-ssse3 -mno-sse4.1 -mno-sse4.2 -mno-avx -mno-avx2 -mno-avx512f \
-O3 -fvectorize -fslp-vectorize \
-fno-exceptions -fno-rtti -fno-asynchronous-unwind-tables \

# --- 3. EXPLICIT MATHEMATHICS COMPONENTS ---
-fassociative-math -fno-signed-zeros -freciprocal-math \
-ffinite-math-only -fno-trapping-math -ffp-contract=fast \

# --- 4. STRUCTURAL SECTIONS & CLEANUP ---
-ffunction-sections -fdata-sections -fno-ident \
-Xclang -opaque-pointers

#to build only static llc for compiling bc
cmake -G Ninja ../llvm \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DLLVM_TARGETS_TO_BUILD="X86" \
  -DLLVM_BUILD_TOOLS=ON \
  -DLLVM_BUILD_UTILS=OFF \
  -DLLVM_BUILD_RUNTIME=OFF \
  -DLLVM_BUILD_RUNTIMES=OFF \
  -DLLVM_INCLUDE_TESTS=OFF \
  -DLLVM_INCLUDE_EXAMPLES=OFF \
  -DLLVM_INCLUDE_BENCHMARKS=OFF \
  -DLLVM_INCLUDE_DOCS=OFF \
  -DLLVM_ENABLE_BINDINGS=OFF \
  -DLLVM_ENABLE_UNWIND_TABLES=OFF \
  -DLLVM_ENABLE_BACKTRACES=OFF \
  -DLLVM_ENABLE_TERMINFO=OFF \
  -DLLVM_ENABLE_ZLIB=OFF \
  -DLLVM_ENABLE_ZSTD=OFF \
  -DLLVM_ENABLE_LIBXML2=OFF \
  -DLLVM_BUILD_STATIC=ON \
  -DLINK_POLLY_INTO_TOOLS=OFF \
  -DLLVM_TOOL_LLVM_DRIVER_BUILD=ON \
  -DCMAKE_EXE_LINKER_FLAGS="-static"

```


```c++
// For combining llc and ld
#include <string>
#include <cstring>
#include <iostream>

// Declare the standard entry points exported by the LLVM driver libraries
extern "C" int llvm_llc_main(int argc, char **argv);
extern "C" int lld_main(int argc, char **argv);

int main(int argc, char **argv) {
    if (argc < 1) return 1;

    // Isolate the base filename from the execution path
    std::string program_name = argv[0];
    size_t last_slash = program_name.find_last_of("/");
    if (last_slash != std::string::npos) {
        program_name = program_name.substr(last_slash + 1);
    }

    // Direct multiplex mapping logic (BusyBox Style)
    if (program_name == "llc") {
        return llvm_llc_main(argc, argv);
    } 
    else if (program_name == "ld" || program_name == "ld.lld") {
        // Enforce the standard ELF configuration target vector internally
        return lld_main(argc, argv);
    }

    // Direct fallback interface if executed directly instead of via a symlink
    if (argc > 1) {
        std::string command = argv[1];
        if (command == "llc") {
            return llvm_llc_main(argc - 1, argv + 1);
        } else if (command == "ld" || command == "ld.lld") {
            return lld_main(argc - 1, argv + 1);
        }
    }

    std::cerr << "Puppy Package Manager Multi-Call Backend Compiler Tool\n";
    std::cerr << "Usage: Symlink as 'llc' or 'ld', or run: pman-cc [llc|ld] [args...]\n";
    return 1;
}
```



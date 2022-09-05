## Calling convention overhead.

1. **stack based parameter passing**
 
  * **Inside the calling function** several things need to be done before and after calling the function
    * **Before calling**
      - callee clobbered registers need to be saved somewhere (usually stack)
      - parameter variables need to be pushed onto the stack
      - return address needs to be stored (kinda like a string in a cave)
    * **After calling**
      - callee clobbered registers need to be restored
      - the return value may need to be moved to a more appropriate location
  * **Inside the called function** several things need to be done before executing the function code
      - **(prologue)**
      - Any registers that the function uses that are not designated by the calling convention need to be saved (usually to stack)
      - Any stack based parameters that need to be in registers for various register-only operations will need to be placed in registers.
      - The frame pointer needs to be set or the stack pointer adjustment needs to be calculated.
    * **"mid-logue"**
      - If it calls another function (a.k.a. not a leaf function) all of the requirements of a callee mentioned above will need to be met for each call
      - each level requires that much more swapping around of data
      - this can be even more if a function requires making a system call (which may clobber all registers and use a whole different stack)
    * **(epilogue)**
      - Any registers that the function used that are not designated by the calling convention need to be restored (usually from stack)
      - The return value needs to be stored in the calling convention's designated location
      - The program returns to the saved return address and removes the address

2. **register based parameter passing**
  * Similar to stack based version except parameters are passed in registers, so instead of pushing them to the stack, they need to be shuffled around into the calling convention designate registers. (most compilers will try to minimize the need for unnecessary swapping)
  * If more parameters are used than the calling convention specifies, the rest are placed on the stack and the same restrictions apply.
  * Implementation specific overheads that may be added on a per function basis
    * Debugging info
    * Unwinding info
    * Address space randomization
    * Language specific additions (this)
    * Extra context switching instructions like `vzeroupper`
    * Cache clobbering
      - The called function will most definitely clobber the cache line
      - There is a good chance it won't be in L1 cache or even L2 depending on the binary/library size

## Compilation differences for inlining.

  1. Compilers cannot inline functions from shared libraries.
  2. Compilers only inline static libraries if both are compile with some form of link time optimization enabled.
  3. "inline" is now mostly treated as a compiler suggestion, so you may need to add a compiler specific attribute to force inlining.  This is usually applicable to functions that are called from multiple locations.
  4. even if a library is statically compiled with link time optimization enabled, it still may not get the same level of optimization and inlining as it would if it were compiled as a single compilation unit (i.e. LTO linking against libpng.a and libjpeg.a vs. compiling with stb_image.h)

## Options

  1.  To allow inlining of a function in a shared library you can provide an alternative static inline version of the functions you want to be inlined in your header file (for example)

```c
        #ifdef INLINE_FOOSTUFF
        static inline void foo(void){ /*do stuff*/ }
        static inline void bar(void){ /*do stuff*/ }
        #else
        void foo(void);
        void bar(void);
        #endif
```

  2. You can split out `foo()` an `bar()` above into a static library and compile it with link time optimization.

  3. You can split out `foo()` an `bar()` above into a single header library.

## New Programming Language alternative

  1. Use the compiler to define the calling convention on a per function basis
  2. Leaf functions would be optimized fully and then update the header file with its parameter registers, returns and clobbers.
  3. Other functions would need to navigate around constraints of its leaf functions
  4. Since it doesn't rely on a system specific calling convention, only a small entry point (`_start()`) would be needed outside of a single architecture 

# Example.
```c
    divmod(int,int):(int,int);
    //becomes this on x after build on x86

    divmod(int:eax,int:edx)(int:eax,int:edx):none;
    /* Since div returns the quotient on eax and the remainder on edx,
     * they are the most efficient registers to use for the diviedend and divisor
     * no other registers or memory are clobbered
     */
```

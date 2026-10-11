/*** compute 16 different floating point math operations in ~5 cycles 
 *** it can be scalled down or up for less or more Remez polynomials 
 *** depending on the precision needed
 ***/

// 16 lanes of 32-bit types (64 bytes / 512 bits total)
typedef float    f32x16 __attribute__((vector_size(64)));
typedef int32_t  i32x16 __attribute__((vector_size(64)));

// Map functions directly to lane indices inside our constant vectors
enum MathOp : int32_t {
    OP_IDENTITY = 0,
    OP_EXP      = 1,
    OP_SIN      = 2,
    OP_COS      = 3,
    // ... paths can be scaled up to 16 distinct operations
};

// 16-lane vector definitions containing 1 constant per math function 
// for EACH iteration of the Horner method polynomial degree.
// Unused lanes are zero-padded to fill out the 16-lane register.
inline const f32x16 C4_VEC = { 0.0f, 0.0416f,  0.0f,      0.0416f }; // [Ident, Exp, Sin, Cos, 0, 0...]
inline const f32x16 C3_VEC = { 0.0f, 0.1666f, -0.1666f,   0.0f    };
inline const f32x16 C2_VEC = { 0.0f, 0.5f,     0.0f,     -0.5f    };
inline const f32x16 C1_VEC = { 1.0f, 1.0f,     1.0f,      0.0f    };
inline const f32x16 C0_VEC = { 0.0f, 1.0f,     0.0f,      1.0f    };

// The Portable High-Throughput Heterogeneous SIMD Engine
f32x16 do_math_f32x16(f32x16 input, i32x16 math_op_mask) {
    // 1. Resolve highest degree constant by mapping opcodes to lane constants in-register
    // __builtin_shuffle(V1, Mask) maps each lane of output to V1[Mask[lane]]
    f32x16 v_acc = __builtin_shuffle(C4_VEC, math_op_mask);

    // 2. 4 FMA steps tracking the Horner method. 
    // The coefficients are extracted dynamically without branches or cache accesses.
    v_acc = (v_acc * input) + __builtin_shuffle(C3_VEC, math_op_mask);
    v_acc = (v_acc * input) + __builtin_shuffle(C2_VEC, math_op_mask);
    v_acc = (v_acc * input) + __builtin_shuffle(C1_VEC, math_op_mask);
    v_acc = (v_acc * input) + __builtin_shuffle(C0_VEC, math_op_mask);

    return v_acc;
}

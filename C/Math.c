/*** compute 16 different floating point math operations in ~5 cycles 
 *** it can be scalled down or up for less or more Remez polynomials 
 *** depending on the precision; up to 32 math functions may be used
 ***/
typedef float    f32x16 __attribute__((vector_size(64)));
typedef int32_t  i32x16 __attribute__((vector_size(64)));

// Step 4 constants for functions 0 to 15
inline const f32x16 C4_PART_A = { /* ... 16 constants ... */ };
// Step 4 constants for functions 16 to 31
inline const f32x16 C4_PART_B = { /* ... 16 constants ... */ };

// Step 3 constants...
inline const f32x16 C3_PART_A = { /* ... 16 constants ... */ };
inline const f32x16 C3_PART_B = { /* ... 16 constants ... */ };

f32x16 do_32_math_f32x16(f32x16 input, i32x16 math_op_mask) {
    // Selects across all 32 possible constants inside register execution ports
    f32x16 v_acc = __builtin_shuffle(C4_PART_A, C4_PART_B, math_op_mask);

    // 4 FMA steps executing 16 lanes chosen out of 32 distinct functions
    v_acc = (v_acc * input) + __builtin_shuffle(C3_PART_A, C3_PART_B, math_op_mask);
    v_acc = (v_acc * input) + __builtin_shuffle(C2_PART_A, C2_PART_B, math_op_mask);
    v_acc = (v_acc * input) + __builtin_shuffle(C1_PART_A, C1_PART_B, math_op_mask);
    v_acc = (v_acc * input) + __builtin_shuffle(C0_PART_A, C0_PART_B, math_op_mask);

    return v_acc;
}

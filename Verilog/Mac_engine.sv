// ============================================================================
// HARDWARE-MAPPED SINGLE-CYCLE MULTIPLY-ACCUMULATE (MAC) ENGINE
// Designed to match FPGA DSP Slice architectures (DSP48E1/DSP48E2 primitives).
// ============================================================================

`include "project_defines.svh"

module mac_engine #(
    parameter DATA_WIDTH = 8,
    parameter ACCUM_WIDTH = 16
)(
    input  logic [DATA_WIDTH-1:0]        operand_data,    // Live incoming streaming token
    input  logic [DATA_WIDTH-1:0]        operand_weight,  // Local neuron weight parameter
    input  logic signed [ACCUM_WIDTH-1:0] accum_in,        // Current running sum history
    
    output logic signed [ACCUM_WIDTH-1:0] accum_out       // Updated wavefront matrix value
);

    // Internal intermediate wires to track signed math expansions cleanly
    logic signed [DATA_WIDTH:0]        s_data;
    logic signed [DATA_WIDTH:0]        s_weight;
    logic signed [2*DATA_WIDTH+1:0]    product;

    // Convert unsigned 8-bit tokens into signed values by padding the sign bit with 0
    assign s_data   = {1'b0, operand_data};
    assign s_weight = {1'b0, operand_weight};

    // Combinational single-cycle multiplication logic block
    assign product  = s_data * s_weight;

    // Accumulate the product directly onto the incoming wavefront total
    assign accum_out = accum_in + product[ACCUM_WIDTH-1:0];

endmodule

// ============================================================================
// MODIFIED 6502 CORE CELL FOR MASSIVELY PARALLEL WAVEFRONT TENSOR INFRASTRUCTURE
// Purged of BCD, Stack, and Interrupt Logic.
// Optimized for 1-Cycle Matrix Dataflow Operations at Target 100 GHz.
// ============================================================================

`include "project_defines.svh"

module modified_6502_core #(
    parameter DATA_WIDTH = 8,
    parameter ADDR_WIDTH = 6,   // 64 weight slots per core (2^6)
    parameter ACCUM_WIDTH = 16  // 16-bit accumulator to guarantee zero overflow
)(
    input  logic                    clk,
    input  logic                    reset_n,
    input  logic                    config_mode,     // S-Register State: 1=Provision, 0=Locked Dataflow
    
    // Physical Interconnect Interfaces (8-Way Moore Sub-Set)
    input  logic [DATA_WIDTH-1:0]   in_diag_left,    // Data from Upper-Left Neighbor
    input  logic [DATA_WIDTH-1:0]   in_diag_right,   // Data from Upper-Right Neighbor
    
    output logic [DATA_WIDTH-1:0]   out_diag_left,   // Fired Token to Lower-Left
    output logic [DATA_WIDTH-1:0]   out_diag_right   // Fired Token to Lower-Right
);

    // ------------------------------------------------------------------------
    // Internal Repurposed 6502 Architectural Registers
    // ------------------------------------------------------------------------
    logic signed [ACCUM_WIDTH-1:0]  reg_A;           // Accumulator (Widened to 16-bit)
    logic        [ADDR_WIDTH-1:0]   reg_X;           // Weight Pointer Register X
    logic        [ADDR_WIDTH-1:0]   reg_Y;           // Weight Pointer Register Y

    // Hardware Interface Wires to Local Sub-Modules
    logic [DATA_WIDTH-1:0]          current_weight_A;
    logic [DATA_WIDTH-1:0]          current_weight_B;
    logic signed [ACCUM_WIDTH-1:0]  mac_result_A;
    logic signed [ACCUM_WIDTH-1:0]  mac_result_B;
    // Pipeline Register to split the dual-MAC critical path
    logic signed [ACCUM_WIDTH-1:0]  reg_mac_intermediate;
    logic [DATA_WIDTH-1:0]          reg_in_diag_right;
 

    // ------------------------------------------------------------------------
    // SUB-MODULE: Localized Weight Memory (Unified Block/Distributed RAM)
    // ------------------------------------------------------------------------
    local_weight_mem #(
        .DATA_WIDTH(DATA_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH)
    ) weight_storage (
        .clk(clk),
        .config_write_en(config_mode),
        .write_addr(reg_X),
        .write_data(in_diag_left),                   // Configuration streams through Left Channel
        
        .read_addr_A(reg_X),
        .read_addr_B(reg_Y),
        .read_data_A(current_weight_A),
        .read_data_B(current_weight_B)
    );

    // ------------------------------------------------------------------------
    // SUB-MODULES: Dual Hardware-Mapped Single-Cycle MAC Engines
    // ------------------------------------------------------------------------
    mac_engine #(.DATA_WIDTH(DATA_WIDTH), .ACCUM_WIDTH(ACCUM_WIDTH)) mac_unit_L (
        .operand_data(in_diag_left),
        .operand_weight(current_weight_A),
        .accum_in(reg_A),
        .accum_out(mac_result_A)
    );

    mac_engine #(.DATA_WIDTH(DATA_WIDTH), .ACCUM_WIDTH(ACCUM_WIDTH)) mac_unit_R (
        //.operand_data(in_diag_right),
        .operand_data(reg_in_diag_right),
        .operand_weight(current_weight_B),
        //.accum_in(mac_result_A),         // Cascade accumulate results directly
        .accum_in(reg_mac_intermediate), 
        .accum_out(mac_result_B)
    );

    // ------------------------------------------------------------------------
    // THE WAVEFRONT RELU PIPELINE ACTIVATION (Hardware Multiplexer)
    // If the highest bit of the 16-bit Accumulator is 1 (Negative), clamp to 0.
    // ------------------------------------------------------------------------
    logic signed [ACCUM_WIDTH-1:0] activated_value;
    assign activated_value = (mac_result_B[ACCUM_WIDTH-1] == 1'b1) ? {ACCUM_WIDTH{1'b0}} : mac_result_B;

    // Quantize down from 16-bit internal state to 8-bit output tokens for transit
    logic [DATA_WIDTH-1:0] saturated_output;
    assign saturated_output = (|activated_value[ACCUM_WIDTH-1:DATA_WIDTH-1]) ? {DATA_WIDTH{1'b1}} : activated_value[DATA_WIDTH-1:0];

    // Output assignment maps directly to physical lower diagonal metal traces
    assign out_diag_left  = saturated_output;
    assign out_diag_right = saturated_output;

    // ------------------------------------------------------------------------
    // PROCESSOR CONTROL LOGIC STATE MACHINE
    // ------------------------------------------------------------------------
    always_ff @(posedge clk or negedge reset_n) begin
        if (!reset_n) begin
            reg_A <= {ACCUM_WIDTH{1'b0}};
            reg_X <= {ADDR_WIDTH{1'b0}};
            reg_Y <= {ADDR_WIDTH{1'b0}};
        end else begin
            if (config_mode) begin
                // --- MODE 1: PROVISIONING MODE (S = 0) ---
                // Increments the X pointer register to fill internal weight memory step-by-step
                reg_X <= reg_X + 1'b1;
                reg_Y <= {ADDR_WIDTH{1'b0}};
                reg_A <= {ACCUM_WIDTH{1'b0}};
                reg_mac_intermediate <= {ACCUM_WIDTH{1'b0}};
                reg_in_diag_right    <= {DATA_WIDTH{1'b0}};
            end else begin
                // --- MODE 2: LOCKED DATAFLOW MODE (S = 1) ---
                // The Instruction Decoder is completely bypassed.
                // The core cycles through weight rows automatically to evaluate tensor lines.
                reg_A <= {ACCUM_WIDTH{1'b0}};        // Clear Accumulator cache for the next wavefront
                
                // Store the intermediate math states to break the execution critical path
                reg_mac_intermediate <= mac_result_A;
                reg_in_diag_right    <= in_diag_right;
                
                // Auto-increment tensor weight pointers to keep execution continuous
                if (reg_X >= (64 - 2)) begin
                    reg_X <= {ADDR_WIDTH{1'b0}};
                    reg_Y <= 1'b1;
                end else begin
                    reg_X <= reg_X + 2'd2;           // Dual read paths consume two weights per step
                    reg_Y <= reg_Y + 2'd2;
                end
            end
        end
    end

endmodule

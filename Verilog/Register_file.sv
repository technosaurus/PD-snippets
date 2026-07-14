// ============================================================================
// PURGED AND OPTIMIZED 6502 REGISTER FILE LAYER
// Tracks internal core state, pointer updates, and the system hardware lock.
// ============================================================================

`include "project_defines.svh"

module register_file #(
    parameter ADDR_WIDTH = 6,
    parameter ACCUM_WIDTH = 16
)(
    input  logic                    clk,
    input  logic                    reset_n,
    input  logic                    config_mode,       // Direct S-register lock flag mapping
    
    // Updates from the Master Pipeline
    input  logic                    clear_accum,
    input  logic signed [ACCUM_WIDTH-1:0] next_accum,
    
    // Current Live Register Outputs
    output logic signed [ACCUM_WIDTH-1:0] out_reg_A,
    output logic        [ADDR_WIDTH-1:0]  out_reg_X,
    output logic        [ADDR_WIDTH-1:0]  out_reg_Y
);

    // Physical register flip-flop arrays
    logic signed [ACCUM_WIDTH-1:0]  r_A;
    logic        [ADDR_WIDTH-1:0]   r_X;
    logic        [ADDR_WIDTH-1:0]   r_Y;

    // Connect internal registers directly to continuous output buses
    assign out_reg_A = r_A;
    assign out_reg_X = r_X;
    assign out_reg_Y = r_Y;

    // Synchronous state transition matrix
    always_ff @(posedge clk or negedge reset_n) begin
        if (!reset_n) begin
            r_A <= {ACCUM_WIDTH{1'b0}};
            r_X <= {ADDR_WIDTH{1'b0}};
            r_Y <= {ADDR_WIDTH{1'b0}};
        end else begin
            if (config_mode) begin
                // --- MODE 1: PROVISIONING MODE ---
                r_X <= r_X + 1'b1;         // X steps continuously to fill memory addresses
                r_Y <= {ADDR_WIDTH{1'b0}}; // Y stays dormant during initialization
                r_A <= {ACCUM_WIDTH{1'b0}};
            end else begin
                // --- MODE 2: LOCKED DATAFLOW MODE ---
                if (clear_accum) begin
                    r_A <= {ACCUM_WIDTH{1'b0}}; // Reset cache window for next wave
                end else begin
                    r_A <= next_accum;          // Keep streaming accumulation results
                end

                // Stride through weight array rows two cells at a time
                if (r_X >= (64 - 2)) begin
                    r_X <= {ADDR_WIDTH{1'b0}};
                    r_Y <= 1'b1;
                end else begin
                    r_X <= r_X + 2'd2;
                    r_Y <= r_Y + 2'd2;
                end
            end
        end
    end

endmodule

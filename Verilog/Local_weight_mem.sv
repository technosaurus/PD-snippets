// ============================================================================
// LOCALIZED WEIGHT STORAGE CELL (DUAL-PORT MEMORY)
// Maps directly to FPGA Distributed Look-Up Table (LUT) RAM or Block RAM blocks.
// ============================================================================

`include "project_defines.svh"

module local_weight_mem #(
    parameter DATA_WIDTH = 8,
    parameter ADDR_WIDTH = 6,
    parameter DEPTH = 64
)(
    input  logic                  clk,
    
    // Write Interface (Used exclusively during Startup Mode)
    input  logic                  config_write_en,
    input  logic [ADDR_WIDTH-1:0] write_addr,
    input  logic [DATA_WIDTH-1:0] write_data,
    
    // Dual Read Interface (Used continuously during Active Dataflow Mode)
    input  logic [ADDR_WIDTH-1:0] read_addr_A,
    input  logic [ADDR_WIDTH-1:0] read_addr_B,
    
    output logic [DATA_WIDTH-1:0] read_data_A,
    output logic [DATA_WIDTH-1:0] read_data_B
);

    // Initialize the actual hardware array register block
    logic [DATA_WIDTH-1:0] memory_array [0:DEPTH-1];

    // Synchronous Write Process: Stores incoming weight stream during boot
    always_ff @(posedge clk) begin
        if (config_write_en) begin
            memory_array[write_addr] <= write_data;
        end
    end

    // Asynchronous Read Operations: Guarantees zero-cycle latency access to parameters
    assign read_data_A = memory_array[read_addr_A];
    assign read_data_B = memory_array[read_addr_B];

endmodule

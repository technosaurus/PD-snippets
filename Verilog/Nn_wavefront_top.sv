// ============================================================================
// SYSTEMVERILOG TOP-LEVEL WRAPPER FOR 128-CORE 3D WAVEFRONT NEURAL NETWORK
// Configured as: 2 Stacked Layers of 8x8 Core Matrices
// ============================================================================

module neural_wavefront_top #(
    parameter DATA_WIDTH = 8,
    parameter CORES_PER_LAYER = 64,
    parameter MATRIX_DIM = 8
)(
    input  logic                   clk,
    input  logic                   reset_n,
    input  logic                   config_mode,  // 1 = Provisioning, 0 = Active Dataflow
    
    // External Real-Time Edge Inputs (Feeds Layer 0 Top Edges)
    input  logic [DATA_WIDTH-1:0]  edge_inputs [0:MATRIX_DIM-1],
    
    // External Real-Time Edge Outputs (Harvests from Layer 1 Bottom Edges)
    output logic [DATA_WIDTH-1:0]  edge_outputs [0:MATRIX_DIM-1]
);

    // ------------------------------------------------------------------------
    // Interconnect Wire Arrays for Diagonal Data Paths
    // Format: [Layer][Row][Col][Data]
    // ------------------------------------------------------------------------
    logic [DATA_WIDTH-1:0] diag_left_out  [0:1][0:MATRIX_DIM-1][0:MATRIX_DIM-1];
    logic [DATA_WIDTH-1:0] diag_right_out [0:1][0:MATRIX_DIM-1][0:MATRIX_DIM-1];

    // ------------------------------------------------------------------------
    // 3D Grid Generation Loop
    // ------------------------------------------------------------------------
    generate
        genvar layer, row, col;
        
        for (layer = 0; layer < 2; layer = layer + 1) begin : g_layers
            for (row = 0; row < MATRIX_DIM; row = row + 1) begin : g_rows
                for (col = 0; col < MATRIX_DIM; col = col + 1) begin : g_cols
                    
                    // Local wiring hooks for the individual core instance
                    logic [DATA_WIDTH-1:0] w_in_left;
                    logic [DATA_WIDTH-1:0] w_in_right;
                    
                    // --------------------------------------------------------
                    // INPUT ROUTING LOGIC: Determine where inputs come from
                    // --------------------------------------------------------
                    if (layer == 0) begin
                        // LAYER 0: Inputs come directly from the external edge pins
                        if (row == 0) begin
                            assign w_in_left  = edge_inputs[col];
                            assign w_in_right = edge_inputs[col];
                        end else begin
                            // Interior rows of Layer 0 read from the row above them
                            assign w_in_left  = (col > 0) ? diag_right_out[layer][row-1][col-1] : {DATA_WIDTH{1'b0}};
                            assign w_in_right = (col < MATRIX_DIM-1) ? diag_left_out[layer][row-1][col+1] : {DATA_WIDTH{1'b0}};
                        end
                    end else begin
                        // LAYER 1 (Stacked): Inputs feed diagonally from the layer above (Layer 0)
                        assign w_in_left  = (col > 0) ? diag_right_out[layer-1][row][col-1] : {DATA_WIDTH{1'b0}};
                        assign w_in_right = (col < MATRIX_DIM-1) ? diag_left_out[layer-1][row][col+1] : {DATA_WIDTH{1'b0}};
                    end

                    // --------------------------------------------------------
                    // CORE INSTANTIATION (The 100 GHz Modified 6502 Cell)
                    // --------------------------------------------------------
                    modified_6502_core #(
                        .DATA_WIDTH(DATA_WIDTH)
                    ) core_inst (
                        .clk(clk),
                        .reset_n(reset_n),
                        .config_mode(config_mode),
                        
                        // Diagonal Receiver Links
                        .in_diag_left(w_in_left),
                        .in_diag_right(w_in_right),
                        
                        // Diagonal Sender Links
                        .out_diag_left(diag_left_out[layer][row][col]),
                        .out_diag_right(diag_right_out[layer][row][col])
                    );
                    
                end
            end
        end
    endgenerate

    // ------------------------------------------------------------------------
    // OUTPUT HARVESTING LOGIC
    // Route the final row of Layer 1 to the chip's external edge output pins
    // ------------------------------------------------------------------------
    generate
        genvar i;
        for (i = 0; i < MATRIX_DIM; i = i + 1) begin : g_outputs
            // Combine or assign final outputs from the bottom row of Layer 1
            assign edge_outputs[i] = diag_left_out[1][MATRIX_DIM-1][i];
        end
    endgenerate

endmodule

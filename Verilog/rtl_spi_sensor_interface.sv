// ============================================================================
// SPI SENSOR INTERFACE FOR REFLEX NETWORK
// Converts 1-bit Serial SPI Input (MOSI) into 8-Channel Parallel Wavefronts
// ============================================================================

`include "project_defines.svh"

module spi_sensor_interface #(
    parameter DATA_WIDTH = 8,
    parameter MATRIX_DIM = 8
)(
    // External Physical Hardware Pins connected to the Sensor Device
    input  logic                   sspi_sclk,     // SPI Master Clock from Camera
    input  logic                   sspi_cs_n,     // SPI Chip Select (Active Low)
    input  logic                   sspi_mosi,     // SPI Master Out Slave In (Data Bit)
    
    // Internal Local FPGA System Interconnects
    input  logic                   sys_clk,       // Main FPGA 150 MHz Internal Clock
    input  logic                   sys_reset_n,
    
    // Parallel Output Vectors feeding the Top Edge of your Layer 0 grid
    output logic [DATA_WIDTH-1:0]  grid_edge_inputs [0:MATRIX_DIM-1],
    output logic                   wavefront_valid // High for 1 cycle when data is ready
);

    // Internal serialization registers running on the external SPI clock domain
    logic [7:0]       shift_reg;
    logic [2:0]       bit_counter;
    logic             byte_ready_spi_domain;
    logic [7:0]       assembled_byte;

    // Channels buffer to replicate data across the 8 parallel top-edge wire rails
    logic [DATA_WIDTH-1:0] parallel_buffer [0:MATRIX_DIM-1];

    // ------------------------------------------------------------------------
    // STEP 1: Capture and Deserialize data on the Sensor's Clock Domain
    // ------------------------------------------------------------------------
    always_ff @(posedge sspi_sclk or posedge sspi_cs_n) begin
        if (sspi_cs_n) begin
            shift_reg             <= 8'h00;
            bit_counter           <= 3'd0;
            byte_ready_spi_domain <= 1'b0;
            assembled_byte        <= 8'h00;
        end else begin
            // Shift incoming bits into our 8-bit capture window (MSB First)
            shift_reg <= {shift_reg[6:0], sspi_mosi};
            
            if (bit_counter == 3'd7) begin
                bit_counter           <= 3'd0;
                byte_ready_spi_domain <= 1'b1; // Strobe high to signal a full byte
                assembled_byte        <= {shift_reg[6:0], sspi_mosi};
            end else begin
                bit_counter           <= bit_counter + 1'b1;
                byte_ready_spi_domain <= 1'b0;
            end
        end
    end

    // ------------------------------------------------------------------------
    // STEP 2: Safe Domain Crossing & Parallel Vector Broadcasting
    // Synchronizes data safely from the slow SPI clock into your 150 MHz grid clock
    // ------------------------------------------------------------------------
    logic rdy_sync_0, rdy_sync_1;
    
    always_ff @(posedge sys_clk or negedge sys_reset_n) begin
        if (!sys_reset_n) begin
            rdy_sync_0      <= 1'b0;
            rdy_sync_1      <= 1'b0;
            wavefront_valid <= 1'b0;
            
            for (int i = 0; i < MATRIX_DIM; i++) begin
                grid_edge_inputs[i] <= {DATA_WIDTH{1'b0}};
            end
        end else begin
            // Double-flop synchronizer to eliminate metastability across clock domains
            rdy_sync_0 <= byte_ready_spi_domain;
            rdy_sync_1 <= rdy_sync_0;
            
            // Detect the rising edge of the synchronized ready signal
            if (rdy_sync_0 && !rdy_sync_1) begin
                wavefront_valid <= 1'b1; // Trigger the 1-cycle reflex calculation
                
                // Broadcast the assembled 8-bit sensor token across the input perimeter
                for (int i = 0; i < MATRIX_DIM; i++) begin
                    grid_edge_inputs[i] <= assembled_byte; 
                end
            end else begin
                wavefront_valid <= 1'b0;
            end
        end
    end

endmodule

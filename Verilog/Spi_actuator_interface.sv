// ============================================================================
// SPI ACTUATOR MASTER INTERFACE FOR REFLEX NETWORK
// Serializes Parallel Bottom-Edge Wavefronts out to Physical Control Pins
// ============================================================================
// 
`include "project_defines.svh"

module spi_actuator_interface #(
    parameter DATA_WIDTH = 8,
    parameter MATRIX_DIM = 8
)(
    input  logic                   sys_clk,         // Main FPGA 150 MHz Internal Clock
    input  logic                   sys_reset_n,
    
    // Inputs from the Bottom Edge of Layer 1
    input  logic [DATA_WIDTH-1:0]  grid_edge_outputs [0:MATRIX_DIM-1],
    input  logic                   wavefront_valid, // High for 1 cycle when grid finishes math
    
    // External Physical Hardware Pins leading to the Target Actuator/Motor
    output logic                   mspi_sclk,       // SPI Clock output driven by FPGA
    output logic                   mspi_cs_n,       // SPI Chip Select output (Active Low)
    output logic                   mspi_mosi        // SPI Master Out Data Bit
);

    // Internal State Machine configurations
    typedef enum logic [1:0] { IDLE, TRANSMIT, CLEANUP } state_t;
    state_t state;

    logic [7:0]  shift_reg;
    logic [2:0]  bit_counter;
    logic [3:0]  clk_divider; // Used to generate the slower SPI clock from sys_clk
    logic        spi_clk_reg;

    assign mspi_sclk = spi_clk_reg;

    // ------------------------------------------------------------------------
    // SPI MASTER CONTROL STATE MACHINE
    // ------------------------------------------------------------------------
    always_ff @(posedge sys_clk or negedge sys_reset_n) begin
        if (!sys_reset_n) begin
            state        <= IDLE;
            mspi_cs_n    <= 1'b1;
            mspi_mosi    <= 1'b0;
            spi_clk_reg  <= 1'b0;
            shift_reg    <= 8'h00;
            bit_counter  <= 3'd0;
            clk_divider  <= 4'd0;
        end else begin
            case (state)
                
                IDLE: begin
                    mspi_cs_n   <= 1'b1;
                    mspi_mosi   <= 1'b0;
                    spi_clk_reg <= 1'b0;
                    bit_counter <= 3'd0;
                    clk_divider <= 4'd0;
                    
                    // The exact microsecond the grid outputs data, catch it and lock the lines
                    if (wavefront_valid) begin
                        // For testing, we harvest the output from column channel 0
                        shift_reg <= grid_edge_outputs[0]; 
                        mspi_cs_n <= 1'b0; // Drop chip select to alert target device
                        state     <= TRANSMIT;
                    end
                end

                TRANSMIT: begin
                    // Divide the internal 150 MHz clock down to generate a stable SPI clock
                    if (clk_divider == 4'd7) begin
                        clk_divider <= 4'd0;
                        spi_clk_reg <= ~spi_clk_reg; // Toggle the SPI clock line
                        
                        // Handle data bit shifts on the falling edge of our new clock
                        if (spi_clk_reg == 1'b1) begin
                            // Shift out the next Most Significant Bit (MSB)
                            mspi_mosi <= shift_reg[7];
                            shift_reg <= {shift_reg[6:0], 1'b0};
                            
                            if (bit_counter == 3'd7) begin
                                state <= CLEANUP;
                            end else begin
                                bit_counter <= bit_counter + 1'b1;
                            end
                        end
                    end else begin
                        clk_divider <= clk_divider + 1'b1;
                    end
                end

                CLEANUP: begin
                    // Ensure the final clock cycle finishes cleanly before raising CS_N
                    if (clk_divider == 4'd7) begin
                        spi_clk_reg <= 1'b0;
                        mspi_mosi   <= 1'b0;
                        mspi_cs_n   <= 1'b1; // Raise chip select to terminate link
                        state       <= IDLE;
                    end else begin
                        clk_divider <= clk_divider + 1'b1;
                    end
                end

            endcase
        end
    end

endmodule

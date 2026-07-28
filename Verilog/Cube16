// ====================================================================
// MODULE: mesh_node
// DESCRIPTION: A single cell in the 3D neuromorphic matrix.
//              Pools inputs from 6 nearest neighbors (X±1, Y±1, Z±1),
//              computes a NOR latch, and updates its routing state.
// ====================================================================
module mesh_node (
    input  wire        clk,          // Unified processing clock
    input  wire        rst_n,        // Active-low reset to clear states
    
    // 6-Directional Nearest-Neighbor Inputs
    input  wire        in_xp,        // X + 1 (East)
    input  wire        in_xm,        // X - 1 (West)
    input  wire        in_yp,        // Y + 1 (North)
    input  wire        in_ym,        // Y - 1 (South)
    input  wire        in_zp,        // Z + 1 (Up)
    input  wire        in_zm,        // Z - 1 (Down)
    
    // Node Outputs
    output reg         node_state,   // Current latched state of this node
    output reg  [2:0]  last_route    // Memory of the last active routing vector
);

    // Pool all incoming neighbor traces into a single wire junction
    wire pooled_input = (in_xp | in_xm | in_yp | in_ym | in_zp | in_zm);

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            node_state <= 1'b0;
            last_route <= 3'b000;
        end else begin
            // Execute the Core universal 1-transistor NOR logic
            node_state <= ~pooled_input;

            // State-Directed Dynamic Routing: 
            // If a neighbor triggered us, record which wire direction it came from
            if (pooled_input) begin
                if      (in_xp) last_route <= 3'd1;
                else if (in_xm) last_route <= 3'd2;
                else if (in_yp) last_route <= 3'd3;
                else if (in_ym) last_route <= 3'd4;
                else if (in_zp) last_route <= 3'd5;
                else if (in_zm) last_route <= 3'd6;
            end
        end
    end
endmodule


// ====================================================================
// MODULE: mesh_cube
// DESCRIPTION: Structural 3D matrix connecting 16x16x16 (4096) nodes.
//              Handles boundaries by wrapping edge nodes to zero.
// ====================================================================
module mesh_cube (
    input  wire        clk,
    input  wire        rst_n,
    
    // External Sensor Boundary Inputs (Mapped to Top Wafer: Z = 15)
    input  wire [15:0] sensor_input_x,
    input  wire [15:0] sensor_input_y,
    
    // External Output Drains (Mapped to Bottom Wafer: Z = 0)
    output wire [15:0] drain_output_x,
    output wire [15:0] drain_output_y
);

    // Define the 3D wire grid interconnecting all nodes
    wire node_matrix [0:15][0:15][0:15];

    // Generate variables for the 3D hardware compilation loop
    genvar x, y, z;
    generate
        for (x = 0; x < 16; x = x + 1) begin: gen_x
            for (y = 0; y < 16; y = y + 1) begin: gen_y
                for (z = 0; z < 16; z = z + 1) begin: gen_z
                    
                    // Local intermediate wires to handle edge boundaries safely
                    wire w_xp, w_xm, w_yp, w_ym, w_zp, w_zm;

                    // Boundary Routing Logic: 
                    // If neighbor exists, attach wire. If edge of cylinder, attach to 0.
                    assign w_xp = (x == 15) ? 1'b0 : node_matrix[x+1][y][z];
                    assign w_xm = (x == 0)  ? 1'b0 : node_matrix[x-1][y][z];
                    assign w_yp = (y == 15) ? 1'b0 : node_matrix[x][y+1][z];
                    assign w_ym = (y == 0)  ? 1'b0 : node_matrix[x][y-1][z];
                    
                    // Top Wafer (Z=15) merges regular neighbors with external analog sensor inputs
                    assign w_zp = (z == 15) ? (sensor_input_x[x] & sensor_input_y[y]) : node_matrix[x][y][z+1];
                    assign w_zm = (z == 0)  ? 1'b0 : node_matrix[x][y][z-1];

                    // Instantiate the physical primitive node at coordinate [x][y][z]
                    mesh_node cell (
                        .clk(clk),
                        .rst_n(rst_n),
                        .in_xp(w_xp),
                        .in_xm(w_xm),
                        .in_yp(w_yp),
                        .in_ym(w_ym),
                        .in_zp(w_zp),
                        .in_zm(w_zm),
                        .node_state(node_matrix[x][y][z]),
                        .last_route() // Left open for internal node usage
                    );

                end
            end
        end
    endgenerate

    // Route the quiet, un-driven bottom layer (Z=0) directly to external output registers
    genvar i;
    generate
        for (i = 0; i < 16; i = i + 1) begin: gen_outputs
            assign drain_output_x[i] = node_matrix[i][8][0];  // Mid-plane slice example
            assign drain_output_y[i] = node_matrix[8][i][0];
        end
    endgenerate

endmodule

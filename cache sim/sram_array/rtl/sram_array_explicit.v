/**
 * Explicit SRAM Array for Synthesis
 * Synthesis variant marked for physical design tools
 * 
 * Features:
 *   - Single memory array marked with synthesis directives
 *   - Meant for SRAM compiler extraction
 *   - Keep hierarchy directive for post-silicon analysis
 *   - Synchronous read/write
 * 
 * Note: This is used during physical design and synthesis for:
 *   - SRAM compiler selection
 *   - Power/area estimation
 *   - Actual memory bist and redundancy insertion
 */

`timescale 1ns / 1ps

module sram_array_explicit #(
    parameter DEPTH = 65536,
    parameter WIDTH = 32,
    parameter ADDR_WIDTH = 16
)(
    input  wire                  clk,
    input  wire                  rst_n,
    input  wire [ADDR_WIDTH-1:0] addr,
    input  wire [WIDTH-1:0]       wr_data,
    input  wire                   wr_en,
    output reg [WIDTH-1:0]       rd_data
);

    // Mark as keep_hierarchy for physical design and SRAM compiler
    (* keep_hierarchy = "yes" *)
    (* ram_style = "block" *)
    reg [WIDTH-1:0] array [0:DEPTH-1];
    
    always @(posedge clk) begin
        // Synchronous read
        rd_data <= array[addr];
        
        // Synchronous write
        if (wr_en) begin
            array[addr] <= wr_data;
        end
    end

endmodule

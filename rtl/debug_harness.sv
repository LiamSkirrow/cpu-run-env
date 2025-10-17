// a dummy test top level module

module debug_harness(
    input clk,
    input reset_n,
    input [3:0] debug_cmd,
    input A,
    input B,
    output Z
);

// instantiate our top-level module to be debugged
top top_inst(
    .clk(clk),
    .reset_n(reset_n),
    .A(A),
    .B(A),
    .Z(Z)
);







endmodule
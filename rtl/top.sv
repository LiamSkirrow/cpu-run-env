// a dummy test top level module

module top(
    input clk,
    input reset_n,
    input A,
    input B,
    output Z
);

reg z_reg;

always_ff@(posedge clk, negedge reset_n) begin
    if(!reset_n) begin
        z_reg <= 1'b0;
    end
    else begin
        z_reg <= A ^ B;
    end
end

assign Z = z_reg;

endmodule
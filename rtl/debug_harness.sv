// a dummy test top level module

module debug_harness(
    input clk,
    input reset_n,
    input [3:0] debug_cmd,
    output command_complete,
    output Z   // the dut's main output, to be sent through to the Python UI
);

localparam STATE_IDLE = 4'h0;
localparam STATE_RUN  = 4'h1;
localparam STATE_HALT = 4'h2;
localparam STATE_STEP = 4'h3;

reg [3:0] state, state_next;
reg A, B, comm_comp, comm_comp_next;
// wire Z;

assign command_complete = comm_comp;

// instantiate our top-level module to be debugged
top top_inst(
    .clk(clk),
    .reset_n(reset_n),
    .A(A),
    .B(B),
    .Z(Z)
);

always_ff@(posedge clk, negedge reset_n) begin : fsm_seq
    if(!reset_n) begin
        state <= 4'h0;
    end
    else begin
        state     <= state_next;
        comm_comp <= comm_comp_next;
    end
end

always_comb begin : fsm_comb
    case(state)
        STATE_IDLE : begin
            A = 0;
            B = 0;
            comm_comp_next = 0; // TODO: make this a one-shot signal with a certain clock delay?
            state_next = debug_cmd;
        end
        STATE_RUN : begin
            A = 0;
            B = 1;
            comm_comp_next = 1;
            state_next = STATE_IDLE;
        end
        STATE_HALT : begin
            A = 1;
            B = 0;
            comm_comp_next = 1;
            state_next = STATE_IDLE;
        end
        STATE_STEP : begin
            A = 1;
            B = 1;
            comm_comp_next = 1;
            state_next = STATE_IDLE;
        end
        default : begin
            A = 0;
            B = 0;
            comm_comp_next = 0;
            state_next = STATE_IDLE;
        end

    endcase
end

endmodule
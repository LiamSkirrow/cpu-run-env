// a dummy test top level module

module debug_harness #(
    parameter DUT_INSTANTIATION = 0
)(
    input        clk,
    input        reset_n,
    input        reset_code_rom_n,
    input [3:0]  debug_cmd,
    input [7:0]  code_rom_data_in,
    input [11:0] code_rom_addr_in,
    input        program_rom_mode,
    output       command_complete,
    output       Z   // the dut's main output, to be sent through to the Python UI
);

localparam NUM_INSTRS = 8;
localparam NUM_BYTES  = NUM_INSTRS*4;

reg [3:0] state, state_next;
reg comm_comp, comm_comp_next;
reg [NUM_BYTES-1:0][7:0] code_rom; // TODO: this has to synth to a reg file (I think???) for hardware implementation
reg [31:0]  code_rom_data_out;
wire [31:0] IMEM_ADDRESS_BUS;
wire [11:0] code_rom_addr;

// notify the outside world that the FSM has finished running its command
assign command_complete = comm_comp;

// handle code rom initialisation
always_ff@(posedge clk, negedge reset_code_rom_n) begin : code_rom_ff
    if(!reset_code_rom_n) begin
        code_rom <= 'd0;
    end
    else begin 
        if(program_rom_mode) begin
            code_rom[code_rom_addr] <= code_rom_data_in;
        end
    end
end

assign code_rom_addr = program_rom_mode ? code_rom_addr_in : IMEM_ADDRESS_BUS[11:0];
assign code_rom_data_out = {code_rom[code_rom_addr+12'd3], code_rom[code_rom_addr+12'd2],
                            code_rom[code_rom_addr+12'd1], code_rom[code_rom_addr+12'd0]};

// instantiate our top-level module to be debugged
generate if(DUT_INSTANTIATION == 0) begin : gen_rv_inst
    localparam STATE_IDLE  = 4'h0;
    localparam STATE_RUN   = 4'h1;
    localparam STATE_STEPI = 4'h2;
    localparam STATE_STEPC = 4'h3;

    reg cpu_halt;

    top cpu_top_inst(
        .clk(clk),
        .rst_n(reset_n),
        .halt(cpu_halt),
        .IMEM_DATA_BUS(code_rom_data_out),
        .IMEM_ADDRESS_BUS(IMEM_ADDRESS_BUS),
        .DMEM_READ_WRN(),
        .DMEM_ADDRESS_BUS(),
        .DMEM_DATA_IN_BUS(32'd0),
        .DMEM_DATA_OUT_BUS()
    );

    always_comb begin : fsm_comb
        case(state)
            STATE_IDLE : begin
                cpu_halt = 1'b1;
                comm_comp_next = 1'b0;
                state_next = debug_cmd;
            end
            STATE_RUN : begin
                cpu_halt = 1'b0;
                comm_comp_next = 1'b1; // wait for breakpoint signal
                state_next = STATE_IDLE;
            end
            STATE_STEPI : begin
                cpu_halt = 1'b0;
                comm_comp_next = 1'b0;
                if(1'b1) begin // TODO: need to grab the 'instruction retired' flag signal
                    comm_comp_next = 1'b1;
                    state_next = STATE_IDLE;
                end
            end
            STATE_STEPC : begin
                cpu_halt = 1'b0;
                comm_comp_next = 1'b1;
                state_next = STATE_IDLE;
            end
            default : begin
                comm_comp_next = 1'b0;
                state_next = STATE_IDLE;
            end
        endcase
    end
end
else if(DUT_INSTANTIATION == 1) begin : gen_dummy_inst
    localparam STATE_IDLE = 4'h0;
    localparam STATE_RUN  = 4'h1;
    localparam STATE_HALT = 4'h2;
    localparam STATE_STEP = 4'h3;

    reg A, B;

    top_dummy top_inst(
        .clk(clk),
        .reset_n(reset_n),
        .A(A),
        .B(B),
        .Z(Z)
    );

    always_comb begin : fsm_comb
        case(state)
            STATE_IDLE : begin
                A = 0;
                B = 0;
                comm_comp_next = 1'b0;
                state_next = debug_cmd;
            end
            STATE_RUN : begin
                A = 0;
                B = 1;
                comm_comp_next = 1'b1;
                state_next = STATE_IDLE;
            end
            STATE_HALT : begin
                A = 1;
                B = 0;
                comm_comp_next = 1'b1;
                state_next = STATE_IDLE;
            end
            STATE_STEP : begin
                A = 1;
                B = 1;
                comm_comp_next = 1'b1;
                state_next = STATE_IDLE;
            end
            default : begin
                A = 0;
                B = 0;
                comm_comp_next = 1'b0;
                state_next = STATE_IDLE;
            end
        endcase
    end
end
endgenerate

always_ff@(posedge clk, negedge reset_n) begin : fsm_seq
    if(!reset_n) begin
        state         <= 4'h0;
        comm_comp     <= 1'h0;
    end
    else begin
        state     <= state_next;
        comm_comp <= comm_comp_next;
    end
end

endmodule
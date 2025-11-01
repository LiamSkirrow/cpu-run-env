`timescale 1ns / 1ps

// the top level deug harness for our dut, to be debugged

module debug_harness #(
    parameter DUT_INSTANTIATION = 0
)(
    input        clk,  // CPU DUT clock
    input        hclk, // debug harness clock
    input        reset_n,
    input        reset_code_rom_n,
    input [3:0]  debug_cmd,
    input [7:0]  code_rom_data_in,
    input [11:0] code_rom_addr_in,
    input        program_rom_mode,
    output       command_complete,
    output reg   exit_signal,
    output [31:0][31:0] reg_dump
);

// TODO: UP TO HERE!!!
// - dump all reg values by default, maybe having some config to select which registers get dumped
//   at the conclusion of each stepi/stepc/run
// - format the register dump in big-endian (for human readability) and use a couple of columns so it 
//   doesn't consume the whole display. Print out reg values in hex.
// - v0.1 is pretty much done! Just have to debug the CPU now :)
//   - except for cleaning up the Makefile to support running arbitrary ASM tests, and other bits
//     of cleanup...

localparam NUM_INSTRS = 128;  // include one instruction space at the end for the 'end' meta-instruction
localparam NUM_BYTES  = NUM_INSTRS*4;

reg [3:0] state, state_next;
reg comm_comp, comm_comp_next, exit_signal_next;
reg [(NUM_BYTES-1)+4:0][7:0] code_rom; // TODO: this has to synth to a reg file (I think???) for hardware implementation
wire [31:0] code_rom_data_out;
wire [31:0] IMEM_ADDRESS_BUS;
wire [11:0] code_rom_addr;
wire        breakpoint_fired;
wire        instruction_retired;
wire        finish_exec_signal;
wire        Z;
wire        unrecognised_opcode_flag;

// notify the outside world that the FSM has finished running its command
assign command_complete = comm_comp;

// handle code rom initialisation
always_ff@(posedge hclk or negedge reset_code_rom_n) begin : code_rom_ff
    if(!reset_code_rom_n) begin
        code_rom <= 'hFF; // TODO: annoyingly, this doesn't seem to work? Manually insert kill instruction in main_tb.cpp instead
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
    localparam STATE_LOAD  = 4'h4;

    reg cpu_halt;

    top cpu_top_inst(
        .clk(clk),
        .rst_n(reset_n),
        .halt(cpu_halt),
        .IMEM_DATA_BUS(code_rom_data_out),    // TODO: this only has to be 8 bits since we read in instructions byte-by-byte
        .IMEM_ADDRESS_BUS(IMEM_ADDRESS_BUS),
        .DMEM_READ_WRN(),
        .DMEM_ADDRESS_BUS(),
        .DMEM_DATA_IN_BUS(32'd0),
        .DMEM_DATA_OUT_BUS(),
        .breakpoint_fired(breakpoint_fired),
        .instruction_retired(instruction_retired),
        .finish_exec_signal(finish_exec_signal),
        .reg_dump_debug(reg_dump),
        .unrecognised_opcode_flag(unrecognised_opcode_flag)
    );

    always_comb begin : fsm_comb
        exit_signal_next = 1'b0;
        cpu_halt         = 'h0;
        comm_comp_next   = 'h0;
        state_next       = 'h0;

        case(state)
            STATE_IDLE : begin
                cpu_halt       = 1'b0;  // counterintuitive, but we're only ever here for 1 cycle
                comm_comp_next = 1'b0;
                state_next     = debug_cmd;
            end
            STATE_RUN : begin
                if(breakpoint_fired) begin
                    cpu_halt       = 1'b1;
                    comm_comp_next = 1'b1;
                    state_next     = STATE_IDLE;
                end
                else begin
                    cpu_halt       = 1'b0;
                    comm_comp_next = 1'b0;
                    state_next     = STATE_RUN;
                    // include an illegal opcode detect here to stop it from infinite looping
                end

                if(finish_exec_signal) begin
                    exit_signal_next = 1'b1;
                    comm_comp_next   = 1'b1;
                end
            end
            STATE_STEPI : begin
                if(instruction_retired) begin
                    cpu_halt       = 1'b1;
                    comm_comp_next = 1'b1;
                    state_next     = STATE_IDLE;
                end
                else begin
                    cpu_halt       = 1'b0;
                    comm_comp_next = 1'b0;
                    state_next     = STATE_STEPI;
                end

                if(finish_exec_signal) begin
                    exit_signal_next = 1'b1;
                    comm_comp_next   = 1'b1;
                end
            end
            STATE_STEPC : begin
                cpu_halt       = 1'b1;
                comm_comp_next = 1'b1;
                state_next     = STATE_IDLE;

                if(finish_exec_signal) begin
                    exit_signal_next = 1'b1;
                    comm_comp_next   = 1'b1;
                end
            end
            default : begin
                cpu_halt       = 1'b1;
                comm_comp_next = 1'b0;
                state_next     = STATE_IDLE;
            end
        endcase
    end
end
else if(DUT_INSTANTIATION == 1) begin : gen_dummy_inst
    // these params don't really mean anything anymore...
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
        exit_signal   <= 1'b0;
    end
    else begin
        state       <= state_next;
        comm_comp   <= comm_comp_next;
        if(exit_signal_next)
            exit_signal <= 1'b1;
    end
end

endmodule
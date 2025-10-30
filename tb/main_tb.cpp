#include <stdlib.h>
#include <iostream>
#include <verilated.h>
#include <verilated_fst_c.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../obj_dir/Vdebug_harness.h"
#include "../obj_dir/Vdebug_harness___024root.h"
#include "../obj_dir/Vdebug_harness__Syms.h"

#include <iomanip>

#define MAX_SIM_TIME         10000
#define MAX_USER_BINARY_SIZE_INSTRS 128
#define MAX_USER_BINARY_SIZE_BYTES  MAX_USER_BINARY_SIZE_INSTRS*4

int mem_idx;
char buffer[8] = { 0 };
unsigned char binary_file_buff[MAX_USER_BINARY_SIZE_BYTES+4] = { 0 };
vluint64_t sim_time = 0;
// HOST = '127.0.0.1'
// PORT = 65432

// TODO: can we include the name of the top level module in argv???
//       if so, we can pass it from the Makefile to here, through to the RTL
//       to set some parameter so that the debug_harness knows *which* dut to instantiate...

int main(int argc, char** argv, char** env) {
    
    // Verilator boilerplate
    Vdebug_harness *dut = new Vdebug_harness;
    Verilated::traceEverOn(true);
    VerilatedFstC *m_trace = new VerilatedFstC;
    dut->trace(m_trace, 5);
    m_trace->open("debug_harness_waves.fst");

    // connect to socket, bound to by Python debug environment
    // creating socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    // specifying address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(65432);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // repeatedly sending connection request to Python server, wait for it to start running
    do{
        // std::cout << "waiting for server to startup...";
    } while(connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)));

    while (sim_time < MAX_SIM_TIME) {

        // block until we receive a trigger codeword from the Python env over the socket
        // TODO: need to validate connection first
        
        ////////////////
        // STEP 1: translate debug cmd into encoded nibble
        ////////////////

        buffer[8] = { 0 };
        while(true){
            recv(clientSocket, buffer, sizeof(buffer), 0);
            // TODO: consolidate these into the same line in a single if statement, cleaner...
            if(!std::strncmp(buffer, "cmd-runn", 8)){
                dut->debug_cmd = 0x1;
                // std::cout << "Got sent cmd-runn" << std::endl;
                break;
            }
            else if(!std::strncmp(buffer, "cmd-stpi", 8)){
                dut->debug_cmd = 0x2;
                // std::cout << "Got sent cmd-step" << std::endl;
                break;
            }
            else if(!std::strncmp(buffer, "cmd-stpc", 8)){
                dut->debug_cmd = 0x3;
                // std::cout << "Got sent cmd-step" << std::endl;
                break;
            }
            else if(!std::strncmp(buffer, "cmd-load", 8)){
                dut->debug_cmd = 0x4;
                // std::cout << "Got sent cmd-load" << std::endl;
                break;
            }
            else if(!std::strncmp(buffer, "cmd-exit", 8)){
                // std::cout << "Got sent cmd-exit" << std::endl;
                break;
            }
        }
        // command exit, time to break the infinite loop, close the trace and finish up
        if(!std::strncmp(buffer, "cmd-exit", 8)){
            std::cout << "Terminating TB" << std::endl;
            break;
        }
        // command load, read in the user binary and pre-initialise the tb's code buffer, without a trace...
        if(!std::strncmp(buffer, "cmd-load", 8)){
            recv(clientSocket, binary_file_buff, MAX_USER_BINARY_SIZE_BYTES, 0);
            // toggle reset, feed in machine code byte-by-byte, toggling clock as we do so
            dut->reset_code_rom_n = 0;
            dut->hclk ^= 1;
            dut->eval();
            m_trace->dump(sim_time);
            sim_time++;

            dut->reset_n = 1;
            dut->program_rom_mode = 1;
            dut->reset_n = 1;
            dut->reset_code_rom_n = 1;
            dut->hclk ^= 1;
            dut->eval();
            // TODO: make the ROM programming sim tracing an optional flag... super useful :D
            m_trace->dump(sim_time);
            sim_time++;
            for(mem_idx = 0; mem_idx < MAX_USER_BINARY_SIZE_BYTES; mem_idx++){
                dut->code_rom_addr_in = mem_idx;
                dut->code_rom_data_in = binary_file_buff[mem_idx];
                // one full clock cycle
                dut->hclk ^= 1;
                dut->eval();
                m_trace->dump(sim_time);
                sim_time++;
                dut->hclk ^= 1;
                dut->eval();
                m_trace->dump(sim_time);
                sim_time++;
                if(mem_idx % 4 == 0 && dut->code_rom_data_in == (unsigned char)0x00){
                    break;
                }
            }
            // insert the final 'done executing' meta-instruction opcode immediately after user ASM
            dut->code_rom_addr_in = mem_idx;
            dut->code_rom_data_in = (unsigned char)0xFF;
            // one full clock cycle
            dut->hclk ^= 1;
            dut->eval();
            m_trace->dump(sim_time);
            sim_time++;
            dut->hclk ^= 1;
            dut->eval();
            m_trace->dump(sim_time);
            sim_time++;

            dut->reset_n = 0;
            dut->hclk ^= 1;
            dut->eval();
            m_trace->dump(sim_time);
            sim_time++;
            
            dut->reset_n = 1;

            dut->hclk ^= 1;
            dut->eval();
            m_trace->dump(sim_time);
            sim_time++;

            dut->program_rom_mode = 0;
            continue;
        }

        // TODO:
        // to handle the bin loading... add a new 'cmd-load' above and include an if() here
        // need to populate a buffer and then exit with 'continue' to restart the sim while loop

        ////////////////
        // STEP 2: wait for debug_harness FSM to complete its operation (poll command_complete flag)
        ////////////////

        while(true){
            
            // TODO: this doesn't do anything since sim_time != 0 at this point
            // release reset on falling edge of clock
            if(sim_time == 0 && dut->clk == 0)
                dut->reset_n = 0;
            else{
                dut->reset_n = 1;
            }

            dut->clk ^= 1;
            dut->eval();
            m_trace->dump(sim_time);
            sim_time++;

            // the debug harness has completed the operation, give a half clock cycle 
            // and then break out of loop
            if(dut->command_complete){
                dut->clk ^= 1;
                dut->eval();
                m_trace->dump(sim_time);
                sim_time++;
                
                break;
            }
        }

        // check if we received the 'finished program' signal, meaning we've finished the program
        if(dut->exit_signal){
            std::cout << "Program finished executing... Exiting Verilator env." << std::endl;
            send(clientSocket, "cmd-exit-resp", sizeof("cmd-exit-resp"), 0);
            break;
        }

        ////////////////
        // STEP 3: transmit relevant output data over socket to Python UI, for display to user (if single-step or hit breakpoint)
        ////////////////

        // acknowledge successful completion of command to Python UI
        if(!std::strncmp(buffer, "cmd-stpi", 8)){
            // std::cout << "Sim is sending resp command..." << std::endl;
            if(send(clientSocket, "cmd-stpi-resp", sizeof("cmd-stpi-resp"), 0) == -1){
                std::cout << "BUG: Failed to send the response command... Exiting. Please create a GitHub Issue!" << std::endl;
                exit(-1);
            }
            send(clientSocket, &dut->reg_dump[14], sizeof(dut->reg_dump[14]), 0);
        }
        else if(!std::strncmp(buffer, "cmd-stpc", 8)){
            if(send(clientSocket, "cmd-stpc-resp", sizeof("cmd-stpc-resp"), 0) == -1){
                std::cout << "BUG: Failed to send the response command... Exiting. Please create a GitHub Issue!" << std::endl;
                exit(-1);
            }
            send(clientSocket, &dut->reg_dump[14], sizeof(dut->reg_dump[14]), 0);
        }
        else if(!std::strncmp(buffer, "cmd-load", 8)){
            if(send(clientSocket, "cmd-load-resp", sizeof("cmd-load-resp"), 0) == -1){
                std::cout << "BUG: Failed to send the response command... Exiting. Please create a GitHub Issue!" << std::endl;
                exit(-1);
            }
            send(clientSocket, &dut->reg_dump[14], sizeof(dut->reg_dump[14]), 0);
        }
        else if(!std::strncmp(buffer, "cmd-runn", 8)){
            if(send(clientSocket, "cmd-runn-resp", sizeof("cmd-runn-resp"), 0) == -1){
                std::cout << "BUG: Failed to send the response command... Exiting. Please create a GitHub Issue!" << std::endl;
                exit(-1);
            }
            send(clientSocket, &dut->reg_dump[14], sizeof(dut->reg_dump[14]), 0);
        }
    }

    // closing socket
    close(clientSocket);

    m_trace->close();
    delete dut;
    exit(EXIT_SUCCESS);
}
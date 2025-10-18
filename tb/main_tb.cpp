#include <stdlib.h>
#include <iostream>
#include <verilated.h>
#include <verilated_fst_c.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../obj_dir/Vdebug_harness.h"
#include "../obj_dir/Vdebug_harness___024root.h"
#include "../obj_dir/Vdebug_harness__Syms.h"

#define MAX_SIM_TIME 10000
char buffer[8] = { 0 };
vluint64_t sim_time = 0;
// HOST = '127.0.0.1'
// PORT = 65432

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
                std::cout << "Got sent cmd-runn" << std::endl;
                break;
            }
            else if(!std::strncmp(buffer, "cmd-halt", 8)){
                dut->debug_cmd = 0x2;
                std::cout << "Got sent cmd-halt" << std::endl;
                break;
            }
            else if(!std::strncmp(buffer, "cmd-step", 8)){
                dut->debug_cmd = 0x3;
                std::cout << "Got sent cmd-step" << std::endl;
                break;
            }
            else if(!std::strncmp(buffer, "cmd-exit", 8)){
                std::cout << "Got sent cmd-exit" << std::endl;
                break;
            }
        }
        if(!std::strncmp(buffer, "cmd-exit", 8)){
            std::cout << "Terminating TB" << std::endl;
            break;
        }

        ////////////////
        // STEP 2: wait for debug_harness FSM to complete its operation (poll command_complete flag)
        ////////////////

        while(true){

            // handle initial reset condition
            if(sim_time == 0 && dut->clk == 0)
                dut->reset_n = 0;
            else{
                dut->reset_n = 1;
            }

            dut->clk ^= 1;
            dut->eval();
            m_trace->dump(sim_time);
            sim_time++;

            // the debug harness has completed the operation, give one *full* extra cycle (to return 
            // to the reset state properly) and then break out of loop
            if(dut->command_complete){
                dut->clk ^= 1;
                dut->eval();
                m_trace->dump(sim_time);
                sim_time++;
                
                break;
            }
        }

        ////////////////
        // STEP 3: transmit relevant output data over socket to Python UI, for display to user (if single-step or hit breakpoint)
        ////////////////

        // acknowledge successful completion of command to Python UI
        if(!std::strncmp(buffer, "cmd-step", 8)){
            std::cout << "Sim is sending resp command..." << std::endl;
            if(send(clientSocket, "cmd-step-resp", sizeof("cmd-step-resp"), 0) == -1){
                std::cout << "BUG: Failed to send the response command... Exiting. Please create a GitHub Issue!" << std::endl;
                exit(-1);
            }
            send(clientSocket, &dut->Z, sizeof(dut->Z), 0);
        }
        else if(!std::strncmp(buffer, "cmd-runn", 8)){
            if(send(clientSocket, "cmd-runn-resp", sizeof("cmd-runn-resp"), 0) == -1){
                std::cout << "BUG: Failed to send the response command... Exiting. Please create a GitHub Issue!" << std::endl;
                exit(-1);
            }
            send(clientSocket, &dut->Z, sizeof(dut->Z), 0);
        }
        else if(!std::strncmp(buffer, "cmd-halt", 8)){
            if(send(clientSocket, "cmd-halt-resp", sizeof("cmd-halt-resp"), 0) == -1){
                std::cout << "BUG: Failed to send the response command... Exiting. Please create a GitHub Issue!" << std::endl;
                exit(-1);
            }
            send(clientSocket, &dut->Z, sizeof(dut->Z), 0);
        }
    }

    // closing socket
    close(clientSocket);

    m_trace->close();
    delete dut;
    exit(EXIT_SUCCESS);
}
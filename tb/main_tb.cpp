#include <stdlib.h>
#include <iostream>
#include <verilated.h>
#include <verilated_fst_c.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../obj_dir/Vtop.h"
#include "../obj_dir/Vtop___024root.h"
#include "../obj_dir/Vtop__Syms.h"

#define MAX_SIM_TIME 20
vluint64_t sim_time = 0;
// HOST = '127.0.0.1'
// PORT = 65432

int main(int argc, char** argv, char** env) {
    
    // Verilator boilerplate
    Vtop *dut = new Vtop;
    Verilated::traceEverOn(true);
    VerilatedFstC *m_trace = new VerilatedFstC;
    dut->trace(m_trace, 5);
    m_trace->open("top_waves.fst");

    // connect to socket, bound to by Python debug environment
    // creating socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    // specifying address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(65432);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    // sending connection request
    connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // block until we receive the trigger codeword from the Python env over the socket
    // TODO: need to validate connection first
    char buffer[8] = { 0 };
    while(true){
        recv(clientSocket, buffer, sizeof(buffer), 0);
        // std::cout << "Message from server: " << buffer << std::endl;
        if(!std::strncmp(buffer, "cmd-runn", 8)){
            std::cout << "Received sim start command from server" << std::endl;
            break;
        }
    }

    while (sim_time < MAX_SIM_TIME) {

        if(sim_time == 0 && dut->clk == 0)
            dut->reset_n = 0;
        else{
            dut->reset_n = 1;
            dut->A = 0;
            dut->B = 1;
        }

        std::cout << "Simulation output: Z = " << (int)dut->Z << std::endl;

        dut->clk ^= 1;
        dut->eval();
        m_trace->dump(sim_time);
        sim_time++;
    }

    // closing socket
    close(clientSocket);

    m_trace->close();
    delete dut;
    exit(EXIT_SUCCESS);
}
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

    // repeatedly sending connection request to Python server, wait for it to start running
    do{
        // std::cout << "waiting for server to startup...";
    } while(connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)));

    // block until we receive the trigger codeword from the Python env over the socket
    // TODO: need to validate connection first
    char buffer[8] = { 0 };
    while(true){
        recv(clientSocket, buffer, sizeof(buffer), 0);
        // std::cout << "Message from server: " << buffer << std::endl;
        if(!std::strncmp(buffer, "cmd-runn", 8)){
            // std::cout << "Received sim start command from server" << std::endl;
            break;
        }
    }

    // TODO: up to here!!!
    // the next thing to do now is to move the above recv() code into the below simulation main loop
    // so that every sim cycle, we:
    // - block (using a while(true)) on the next command to be sent over from the Python env
    // - when we receive a command we then break out of the infinite loop and update the dut->command (or whatever) 
    //   port to communicate to the debug harness RTL that it needs to run/step/whatever. We don't need to validate the 
    //   command received from the Python env since it's already validated when the user types it in on the cmd prompt.
    //   ...
    // - Ok... actually maybe it is useful to record the fst trace whilst in this python controlled mode... We can manually
    //   run dut->eval() ourselves so it won't be recording all the deadtime whilst waiting for the Python cmds to come in.
    // - play with the existing trivial module for now rather than instantiating the rv-CPU. Include debug harness FSM states 
    //   like setA, resetA, setB, resetB etc... That way we can see how the fst will look, and we can play around with returning
    //   the value of z_reg to the Python env each clock cycle. We can also implement free run and single-step mode with this...

    while (sim_time < MAX_SIM_TIME) {

        if(sim_time == 0 && dut->clk == 0)
            dut->reset_n = 0;
        else{
            dut->reset_n = 1;
            dut->A = 0;
            dut->B = 1;
        }

        // std::cout << "Simulation output: Z = " << (int)dut->Z << std::endl;

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
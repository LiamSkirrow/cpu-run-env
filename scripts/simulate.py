# interact with the simulation environment via a socket connection

import socket
import time

HOST = '127.0.0.1'
PORT = 65432

# handle the user input, dispatch testbench commands
def handleCommands(conn):
    while(True):
        user_in = input('cpu-virtual-debugger $> ')
        if(user_in == 'exit'):
            conn.send(b'cmd-exit')
            break
        elif(user_in == 'help'):
            # TODO: print out a helpful summary of commands with a decsription for each of them
            pass
        elif(user_in == 'run'):
            conn.send(b'cmd-runn')
        elif(user_in == 'step'):   # TODO: split into stepc (step clock) or stepi (step instruction)
            conn.send(b'cmd-step')
        elif(user_in == 'halt'):
            conn.send(b'cmd-halt')
        elif(user_in == '\n'): # TODO: figure out why this isn't working
            pass
        else:
            print('Unrecognised command. Type \'help\' for available commands...')

        # now block waiting on response command from TB
        if(user_in == 'run' or user_in == 'step' or user_in == 'halt'):
            resp = conn.recv(14)
            print('Received value of resp from sim: ' + str(resp) + ' with size: ' + str(len(resp)))
            if(resp == b'cmd-runn-resp\0' or resp == b'cmd-step-resp\0' or resp == b'cmd-halt-resp\0'):
                # resp = conn.recv(4)
                print('Value of Z: ')# + str(resp))
            else:
                print('Got an incorrect response command from simulation... Bug detected! Please report on GitHub Issues...')
                exit(0)

# open a socket connection and wait until the client connects
def initConnection():
    print('Opening a socket on HOST/PORT: ' + HOST + '/' + str(PORT) + '...')
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        conn, addr = s.accept()
        print(f"Connection from client {addr}")
    return conn
            
if __name__ == "__main__":
    conn = initConnection()
    # TODO: I'm getting some crashes when failing to connect, I think that I'm not closing the socket correctly

    handleCommands(conn)
    conn.close()

    # conn.send(b'beginnnn')
    # with open('gen-output/dummy.genbin', 'rb') as userBinary:
    #     for line in userBinary:
    #         conn.sendall(line)    

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
        elif(user_in == 'run'):
            conn.send(b'cmd-runn')
        elif(user_in == 'step'):
            conn.send(b'cmd-step')
        elif(user_in == 'halt'):
            conn.send(b'cmd-halt')
        elif(user_in == '\n'):
            pass
        else:
            print('Unrecognised command...')
        
        # TODO:
        # after sending a command, need to wait on the tb sending back the response
        # before reiterating the while() loop

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

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
            break
        elif(user_in == 'run'):
            conn.send(b'cmd-runn')
        else:
            print('Unrecognised command...')

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

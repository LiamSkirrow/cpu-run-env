# interact with the simulation environment via a socket connection

import socket
import time

HOST = '127.0.0.1'
PORT = 65432

def initConnection():
    print('Opening a socket on HOST/PORT: ' + HOST + '/' + str(PORT) + '...')
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        conn, addr = s.accept()
        print(f"Connection from client {addr}")
        # with conn:
        #     while True:
        #         data = conn.recv(1024)
        #         if not data:
        #             break
        #         conn.sendall(data)
    return conn
            
if __name__ == "__main__":
    conn = initConnection()

    # TODO: I'm getting some crashes when failing to connect, I think that I'm not closing the socket correctly

    conn.send(b'beginnnn')
    # with open('gen-output/dummy.genbin', 'rb') as userBinary:
    #     for line in userBinary:
    #         conn.sendall(line)    

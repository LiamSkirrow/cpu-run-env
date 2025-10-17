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
        with conn:
            print(f"Connection from {addr}")
            while True:
                data = conn.recv(1024)
                if not data:
                    break
                conn.sendall(data)
    return conn
            
if __name__ == "__main__":
    conn = initConnection()
    with open('gen-output/dummy.genbin', 'rb') as userBinary:
        for line in userBinary:
            conn.sendall(line)
        
    # TODO: NEXT!
    # connect to this socket connection via the testbench. It's Verilator time!!!
    


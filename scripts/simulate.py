# interact with the simulation environment via a socket connection

import socket
from argparse import ArgumentParser

HOST = '127.0.0.1'
PORT = 65432

# handle input args
parser = ArgumentParser()
parser.add_argument("-t", "--testname", required=True, type=str, help="Give the name of the test to be run, used to lookup the relevant bin")

args = parser.parse_args()

test_name = args.testname
test_path = 'gen-output/' + test_name + '.genbin'

# print out the latest state of the CPU registers
def formatRegDump(regs):
    for i in range(0,8):
        reg_index = i
        # print out the 8 rows of 4 columns
        print('x' + str(reg_index)      + ': ' + str(regs[4*reg_index      : (4*reg_index)+4]),      end='   ')
        print('x' + str(reg_index + 8)  + ': ' + str(regs[4*reg_index + 8*4  : (4*reg_index + 8*4)+4]),  end='   ')
        print('x' + str(reg_index + 16) + ': ' + str(regs[4*reg_index + 16*4 : (4*reg_index + 16*4)+4]), end='   ')
        print('x' + str(reg_index + 24) + ': ' + str(regs[4*reg_index + 24*4 : (4*reg_index + 24*4)+4]))

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

        elif(user_in == 'stepi'):   # TODO: split into stepc (step clock) or stepi (step instruction)
            conn.send(b'cmd-stpi')

        elif(user_in == 'stepc'):
            conn.send(b'cmd-stpc')

        elif(user_in == 'load'):
            # don't do this
            # conn.send(b'cmd-load')
            # do do this, but figure out how to do it right
            # load_bin(conn, test_path)
            pass

        elif(user_in == '\n'): # TODO: figure out why this isn't working
            pass
        # elif(user_in == 'load [testname]'):
        #     TODO: load a new binary instead of the old one, overwrite the buffer completely
        else:
            print('Unrecognised command. Type \'help\' for available commands...')

        # now block waiting on response command from TB
        if(user_in == 'run' or user_in == 'stepi' or user_in == 'stepc'):
            resp = conn.recv(14)
            # print('Received value of resp from sim: ' + str(resp) + ' with size: ' + str(len(resp)))
            if(resp == b'cmd-exit-resp\0'):
                resp = conn.recv(128)
                formatRegDump(resp)                
                print('Received exit response... Exiting Python env')
                break
            elif(resp == b'cmd-runn-resp\0' or resp == b'cmd-stpi-resp\0' or resp == b'cmd-stpc-resp\0' or resp == b'cmd-load-resp\0'):
                resp = conn.recv(128)
                formatRegDump(resp)
            else:
                print('Got an incorrect response command from simulation... Bug detected! Please report on GitHub Issues...')
                exit(0)
        elif(user_in == 'load'):
            pass
            # send a new binary file, same as below (can the code be consolidated?)
        
        # TODO: detect if we've reached the end of the ASM code and exit accordingly
        # ->    if we receive the exit_signal kill signal. Exit gracefully...

# open a socket connection and wait until the client connects
def initConnection():
    print('Opening a socket on HOST/PORT: ' + HOST + '/' + str(PORT) + '...')
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        conn, addr = s.accept()
        print(f"Connection from client {addr}")
    return conn

def load_bin(conn, test_path):
    conn.send(b'cmd-load')
    with open(test_path, 'rb') as userBinary:
        for line in userBinary:
            conn.send(line)
            
if __name__ == "__main__":
    # init connection
    conn = initConnection()
    # send the binary over the socket connection
    load_bin(conn, test_path)
    # handle user input and run commands in the main control loop
    handleCommands(conn)
    conn.close()


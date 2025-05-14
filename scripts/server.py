import socket
import threading
import time 

PORT = 5050
SERVER = socket.gethostbyname(socket.gethostname())
HEADER = 16
FORMAT = "utf-8"
DISCONNECT_MSG = "!DISCONNECT"
ADDR = (SERVER, PORT)
TIMEOUT = 60

server = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM, proto=socket.IPPROTO_IP)
server.settimeout(TIMEOUT)

server.bind(ADDR)


def handle_client(conn, addr):
    print(f"[NEW CONNECTION] {addr} connected.") 
    
    connected = True
    while connected:
        msg_raw = conn.recv(HEADER)
        print("================================")
        print(f"[RAW MESSAGE]: {msg_raw}")
        msg_length = int(msg_raw.decode().replace('\x00', '').strip())
        if msg_length:
            msg_length = int(msg_length)

            msg = conn.recv(msg_length).decode(FORMAT)
            print(f"[RECEIVED], message: {msg}, size: {msg_length}")
            if msg == DISCONNECT_MSG:
                print(f"[DISCONNECT]")
                connected = False
            print(f"{addr}: {msg}")

    conn.close()
    exit()


# def handle_keyboard():
#     while True:
#         time.sleep(1)
#         print("test")
#         # server.close()
#         # exit


def start():
    try:
        server.listen()
    except TimeoutError:
        print("timeout error")
        return 
    except Exception as e:
        print(f"Unexpected error: {e}")
        return 

    # thread_keyboard= threading.Thread(target=handle_keyboard)
    # thread_keyboard.start()

    print(f"[LISTENING], sever is listening on {SERVER}:{PORT}")
    while True:
        try:
            conn, addr = server.accept()
            thread_server= threading.Thread(target=handle_client, args=(conn, addr))
            thread_server.start()

            print(f"[ACTIVE CONNECTIONS] {threading.active_count() - 1}")   
        except socket.timeout:
            exit()


print("[STARTING], server is starting ...")
start()
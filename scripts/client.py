import socket
import time 

PORT = 5050
SERVER = socket.gethostbyname(socket.gethostname())
# SERVER = "192.168.1.4"
HEADER = 16
FORMAT = "utf-8"
DISCONNECT_MSG = "!DISCONNECT"
ADDR = (SERVER, PORT)





def send(msg):
    message = msg.encode(FORMAT)
    msg_length = len(message)
    send_length = str(msg_length).encode(FORMAT)
    # pad the message
    send_length += b' ' * (HEADER - len(send_length))

    client.send(send_length)
    print(f"sent string: {send_length}")
    client.send(message)
    print(f"sent string: {message}")



print(ADDR)
client = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)
client.connect(ADDR)
# while True:
print(ADDR)
send("Hello worl1!")
input()
send("Hello world2!")
input()
send("Hello world3!")
send(DISCONNECT_MSG)


import socket

PORT = 5050
SERVER = socket.gethostbyname(socket.gethostname())
HEADER = 64
FORMAT = "utf-8"
DISCONNECT_MSG = "!DISCONNECT"
ADDR = (SERVER, PORT)


client = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)
client.connect(ADDR)

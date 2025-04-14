import socket
HOST = '0.0.0.0'
PORT = 8080


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen()
print(f"Listening on port {PORT}...")
conn, addr = s.accept()
with conn:  
    print(f"Connected by {addr}")
    while True:
        data = conn.recv(1024)
        if not data:
            break
        print("Received:", data.decode())


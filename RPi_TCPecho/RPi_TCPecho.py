import socket

HOST = "0.0.0.0"          # Listen on all interfaces
PORT = 5050               # Non‑exotic, unprivileged port
ALLOWED_CLIENT = "192.168.50.13"

def run_server():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.bind((HOST, PORT))
        server.listen(1)
        print(f"Server listening on {HOST}:{PORT}")

        while True:
            conn, addr = server.accept()
            client_ip, client_port = addr
            print(f"Connection from {client_ip}:{client_port}")

            if client_ip != ALLOWED_CLIENT:
                print("Rejected: unauthorized IP")
                conn.close()
                continue

            with conn:
                data = conn.recv(1024)
                if not data:
                    continue

                message = data.decode().strip()
                print(f"Received: {message}")

                reply = f"{message} - received"
                conn.sendall(reply.encode())
                print(f"Replied: {reply}")

if __name__ == "__main__":
    run_server()

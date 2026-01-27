import queue
import socket
import time


# ============================================================
#  TCP Communication Thread
# ============================================================

def tcp_comm_thread(in_q: queue.Queue, out_q: queue.Queue):
    HOST = "0.0.0.0"
    PORT = 5050

    # Create server socket
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((HOST, PORT))
    server.listen(1)
    server.settimeout(0.2)

    print(f"[TCP] Server listening on {HOST}:{PORT}")

    client = None

    while True:
        # ------------------------------------------------------------
        # 1. Handle commands from main (fire-and-forget)
        # ------------------------------------------------------------
        try:
            cmd = in_q.get_nowait()
            match cmd:
                case ("SEND", payload):
                    if client:
                        try:
                            client.sendall(payload.encode())
                        except Exception:
                            client.close()
                            client = None

                case ("CLOSE",):
                    if client:
                        client.close()
                    server.close()
                    return

                case _:
                    print(f"[TCP] Unknown command: {cmd}")

        except queue.Empty:
            pass

        # ------------------------------------------------------------
        # 2. Accept a client if not connected
        # ------------------------------------------------------------
        if client is None:
            try:
                conn, addr = server.accept()
                conn.settimeout(0.2)
                client = conn
                print(f"[TCP] Client connected: {addr}")
            except socket.timeout:
                pass
            except Exception as e:
                print(f"[TCP] Accept error: {e}")
                time.sleep(0.2)

        # ------------------------------------------------------------
        # 3. Receives a message and disconnects
        # ------------------------------------------------------------

        if client:
            try:
                data = client.recv(1024)
                if data:
                    msg = data.decode().strip()
                    out_q.put(msg)

                # Always disconnect after receiving anything
                client.close()
                client = None

            except socket.timeout:
                pass
            except Exception:
                if client:
                    client.close()
                client = None

        time.sleep(0.01)

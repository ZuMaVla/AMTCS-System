import queue
import socket
import time
from unittest import case
from config import TCPcfg


# ============================================================
#  TCP Communication Thread
# ============================================================

def tcp_comm_thread(in_q: queue.Queue, out_q: queue.Queue):
    REMOTE_HOST = TCPcfg.remote_host
    HOST = TCPcfg.host
    PORT = TCPcfg.port
    TIMEOUT = TCPcfg.timeout
    

    # Create server socket
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((HOST, PORT))
    server.listen(1)
    server.settimeout(TIMEOUT)

    print(f"[TCP] Server listening on {HOST}:{PORT}")

    client = None

    while True:
        # ------------------------------------------------------------
        # 1. Handle commands from the main thread (fire-and-forget)
        # ------------------------------------------------------------
        try:
            cmd = in_q.get_nowait()
            if cmd:
                match cmd:
                    case ("SEND", payload):
                        try:
                            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                                s.settimeout(0.5)
                                s.connect((REMOTE_HOST, PORT)) # <-- send to fixed remote host 
                                s.sendall(payload.encode()) # Do NOT recv(), do NOT wait for reply 
                        except Exception as e:
                            print(f"[TCP] SEND error: {e}")

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
                conn.settimeout(TIMEOUT)
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
                    keyword = msg.split()[0]
                    match keyword:
                        case "PING":
                            out_q.put(("STATUS", "iHR320_OK"))
                            in_q.put(("SEND", "PONG"))
                        case "UPDATE":
                            out_q.put(("REQUEST", "iHR320_UPDATE"))
                        case "CANCEL":
                            out_q.put(("REQUEST", "iHR320_CANCEL")) 
                        case "INIT":   
                            out_q.put(("INITIALISATION", " ".join(msg.split()[1:])))        # pass the temperature list to the PLC
                        case "DONE":
                            out_q.put(("STATUS", "iHR320_DONE"))
                        case "PAUSE":
                            out_q.put(("REQUEST", "USER_PAUSE"))
                        case "RESUME":
                            out_q.put(("REQUEST", "USER_RESUME"))
                        case "ADD":
                            out_q.put(("ADD T", msg.split()[1]))                            # pass additional temperature to the PLC
                        case "REMOVE":
                            out_q.put(("REMOVE T", msg.split()[1]))                         # pass temperature to remove to the PLC
                        case _:
                            print(f"[TCP] Unknown message: {msg}")
                # Always disconnect after receiving anything (no live connection)
                client.close()
                client = None

            except socket.timeout:
                pass
            except Exception:
                if client:
                    client.close()
                client = None

        time.sleep(0.1)

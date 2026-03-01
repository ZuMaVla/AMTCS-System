import queue
import socket
import time
import subprocess
import os
import signal
from unittest import case
from config import TCPcfg


# ============================================================
#  TCP Communication Helpers
# ============================================================


def is_port_in_use(port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.bind(("0.0.0.0", port))
            return False
        except OSError:
            return True


def get_pid_using_port(port):
    try:
        output = subprocess.check_output(["lsof", "-t", f"-i:{port}"])
        return int(output.strip())
    except subprocess.CalledProcessError:
        return None


def kill_process(pid):
    try:
        os.kill(pid, signal.SIGKILL)
        print(f"Killed process {pid} holding the port")
    except Exception as e:
        print(f"Failed to kill process {pid}: {e}")


# ============================================================
#  TCP Communication Thread
# ============================================================

def tcp_comm_thread(in_q: queue.Queue, out_q: queue.Queue):
    REMOTE_HOST = TCPcfg.remote_host
    HOST = TCPcfg.host
    LISTEN_PORT = TCPcfg.LISTEN_PORT
    SEND_PORT = TCPcfg.SEND_PORT
    TIMEOUT = TCPcfg.timeout
    
    pid = get_pid_using_port(LISTEN_PORT)
    if pid: 
        print(f"Port {LISTEN_PORT} is busy, killing PID {pid}") 
        kill_process(pid) 
    else: print(f"Port {LISTEN_PORT} is free")

    time.sleep(1)  # Give some time for the port to be released

    # Create server socket
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((HOST, LISTEN_PORT))
    server.listen(5)
    server.settimeout(TIMEOUT)

    print(f"[TCP] Server listening on {HOST}:{LISTEN_PORT}")

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
                            with socket.create_connection((REMOTE_HOST, SEND_PORT), timeout=2) as s:
                                s.sendall(payload.encode())
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
        # 2. Accept a client if not connected and receive the data they have to send
        # ------------------------------------------------------------
        data = None
        if client is None:
            try:
                conn, addr = server.accept()
                data_fragments = []
                while True:
                    fragment = conn.recv(1024)
                    if not fragment:
                        break
                    data_fragments.append(fragment)
                data = b''.join(data_fragments)

                conn.close()
                conn = None
                print(f"[TCP] Client connected: {addr}")
                print(f"[TCP] Data received from: {addr}")
            except socket.timeout:
                pass
            except ConnectionResetError:
                continue   # totally normal, ignore
            except Exception as e:
                print(f"[TCP] Accept error: {e}")
                
            if data:
                msg = data.decode().strip()
                keyword = msg.split()[0]
                payload = " ".join(msg.split()[1:])
                match keyword:
                    case "PING":
                        out_q.put(("STATUS", "iHR320_OK"))
                        in_q.put(("SEND", "PONG"))
                    case "TC?":
                        out_q.put(("REQUEST", "TC_STATUS"))                             # TC status has been requested
                    case "UPDATE":
                        out_q.put(("REQUEST", "iHR320_UPDATE"))
                    case "CANCEL":
                        out_q.put(("REQUEST", "iHR320_CANCEL")) 
                    case "INIT":   
                        out_q.put(("INITIALISATION", payload))                          # pass the temperature list to the PLC
                    case "DONE":
                        out_q.put(("REPORT", "SPECTRUM_ACQUIRED"))                      # Spectrum has been acquired
                    case "AFFIRMATIVE":
                        out_q.put(("AFFIRMATIVE", "SPECTRUM_REQUESTED"))                # Spectrum request has been received 
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

        time.sleep(1)

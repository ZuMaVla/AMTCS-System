import threading
import queue
import time
from serial_listener import serial_comm_thread
from tcp_listener import tcp_comm_thread
from config import PLCcfg

# ============================================================
#  Main Thread (PLC)
# ============================================================

def main():
    # Queues for TCP communication
    tcp_in = queue.Queue()      # commands → TCP 
    tcp_out = queue.Queue()     # events ← TCP 

    # Queues for Serial communication
    ser_in = queue.Queue()      # commands → Serial 
    ser_out = queue.Queue()     # events ← Serial 

    # Start communication threads
    t_tcp = threading.Thread(target=tcp_comm_thread, args=(tcp_in, tcp_out), daemon=True)
    t_ser = threading.Thread(target=serial_comm_thread, args=(ser_in, ser_out), daemon=True)

    t_tcp.start()
    t_ser.start()

    print("Main event loop running...")

 
    TIMEOUT = PLCcfg.timeout

    # Main event loop
    while True:
        # Handle TCP events
        try:
            (keyword, payload) = tcp_out.get_nowait()
            print(f"[MAIN] TCP event: {keyword}: {payload}")
        except queue.Empty:
            pass

        # Handle Serial events
        try:
            msg = ser_out.get_nowait()
            print(f"[MAIN] SERIAL event: {msg}")
        except queue.Empty:
            pass

        # Periodic tasks or routing logic here
        time.sleep(TIMEOUT)


if __name__ == "__main__":
    main()

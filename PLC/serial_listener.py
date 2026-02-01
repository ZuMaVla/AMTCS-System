import queue
import serial
import time


# ============================================================
#  Serial Communication Thread
# ============================================================

def serial_comm_thread(in_q: queue.Queue, out_q: queue.Queue):
    port="COM1"
    baud_rate=14400
    ser = serial.Serial(port, baudrate=baud_rate, timeout=0.5)

    while True:
        # 1. Handle commands from main thread
        try:
            cmd = in_q.get_nowait()
            match cmd:
                case ("WRITE", payload):
                    ser.write((payload + "\n").encode())
                case ("CLOSE",):
                    ser.close()
                    return
                case _:
                    print(f"[SERIAL] Unknown command: {cmd}")
        except queue.Empty:
            pass

        # 2. Handle incoming serial messages
        try:
            line = ser.readline().decode().strip()
            if line:
                out_q.put(line)
        except Exception as e:
            print(f"[SERIAL] Error: {e}")
            time.sleep(1)


import queue
import serial
import time


# ============================================================
#  Serial Communication Thread
# ============================================================

def serial_comm_thread(in_q: queue.Queue, out_q: queue.Queue):
    ser = serial.Serial(
        port="/dev/ttyUSB0",
        baudrate=9600,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=0.5
    )

    waiting_for_reply = False

    print(f"[SERIAL] Serial port opened on {ser.port}")


    while True:
        # 1. Check for incoming commands and write if not waiting for reply
        if not waiting_for_reply:
            try:
                cmd = in_q.get_nowait()
                match cmd:
                    case ("WRITE", payload):
                        ser.write((payload + "\n").encode())
                        waiting_for_reply = True   # switch to RX/receiver mode
                    case ("CLOSE",):
                        ser.close()
                        return
            except queue.Empty:
                pass

        # 2. If waiting for reply, read incoming data
        if waiting_for_reply:
            line = ser.readline().decode(errors="ignore").strip()
            if line:
                out_q.put(line)
                print(f"[SERIAL] message received on {ser.port}: {line}")
                waiting_for_reply = False

        time.sleep(0.5)

import queue
from threading import Timer
import serial
import time
from config import ExperimentMode


def simulate_serial(state, serial_line):
    state["line"] = serial_line



# ============================================================
#  Serial Communication Thread
# ============================================================

def serial_comm_thread(in_q: queue.Queue, out_q: queue.Queue, exp_mode: ExperimentMode):
    ser = serial.Serial(
        port="/dev/ttyUSB0",
        baudrate=9600,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=0.5
    )

    waiting_for_reply = False
    state = {"line": None}                  # Shared state for simulation mode
    line = None                             # Container for the incoming message  
   
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
                    case ("SIMULATE", payload):
                        timer_T = Timer(0.5, simulate_serial, args=(state, payload))
                        timer_T.start()
                        waiting_for_reply = True   # switch to RX/receiver mode
                    case ("CLOSE",):
                        ser.close()
                        return
            except queue.Empty:
                pass

        # 2. If waiting for reply, read incoming data
        if waiting_for_reply:
            if exp_mode == ExperimentMode.SIMULATION:
                line = state["line"]                                    # In simulation mode, we read from the shared state 
                if line: 
                    state["line"] = None                                # Clear the line after reading    
            else:
                line = ser.readline().decode(errors="ignore").strip()   # In production mode, we read from the serial port
            if line:
                out_q.put(line)
                print(f"[SERIAL] message received on {ser.port}: {line}")
                waiting_for_reply = False
        time.sleep(0.5)

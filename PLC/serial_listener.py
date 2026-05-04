import queue
from threading import Timer
import serial
import time
from config import ExperimentMode


def simulate_serial(state, serial_line):
    state["line"] = serial_line

simulate_serial_state = {"line": None}

last_payload = ""  
emu_init = False 
emu_current_T = "294.000"

def expected_responses(command: str):
    global last_payload
    global emu_init
    global emu_current_T
    match command:
        case "*IDN?":
            emu_init = True
            return "Cryocon Model 32, Rev 6.08H"
        case "LOOP 1:SETPT?":
            print("Last payload:", last_payload, "\n")
            emu_current_T = last_payload
            return last_payload
        case "INPUT? B": 
            return emu_current_T
        case "CONT?": 
            return "ON"
        
class EmuSerial:
    def __init__(self, state_dict, response_func):
        self.state = state_dict
        self.get_response = response_func  # expected response logic 
        self.port = "EMULATED_SERIAL"

    def write(self, data):
        command = data.decode(errors="ignore").strip() 
        print(f"[EMU-serial]: {command}")

        response = self.get_response(command)
        
        if response is not None:
            self.state["line"] = response
            
    def readline(self):
        raw_data = self.state.get("line")

        if raw_data:
            self.state["line"] = None  # empty the 'buffer'
            return (raw_data + "\n").encode("utf-8")
        return b""

    def close(self):
        pass
            
# ============================================================
#  Serial Communication Thread
# ============================================================

def serial_comm_thread(in_q: queue.Queue, out_q: queue.Queue, exp_mode: ExperimentMode):
    global last_payload
    if exp_mode == ExperimentMode.PRODUCTION:
        try:
            ser = serial.Serial(
                port="/dev/ttyUSB0",
                baudrate=9600,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=0.5
            )
        except (serial.SerialException, ImportError):
            print("!!! Serial port not found. Initializing Virtual Serial Port...")
            ser = EmuSerial(simulate_serial_state, expected_responses)
            exp_mode = ExperimentMode.SIMULATION
            out_q.put("SERIAL_PORT_UNAVAILABLE")
    else:  
        ser = EmuSerial(simulate_serial_state, expected_responses)
        
    waiting_for_reply = False
    state = {"line": None}                  # Shared state for simulation mode
    line = None                             # Container for the incoming message  
   
    print(f"[SERIAL] Serial port opened on {ser.port}")
    is_serial_listener_running = True

    while is_serial_listener_running:
        # Check for incoming commands and write if not waiting for reply
        if not waiting_for_reply:
            try:
                cmd = in_q.get_nowait()
                if cmd:
                    match cmd:
                        case ("TC_STATUS", payload):
                            ser.write(("*IDN?" + "\n").encode())
                            waiting_for_reply = True    # switch to receiver mode
                        case ("GO_TO_TEMPERATURE", payload):
                            ser.write(("LOOP 1:SETPT " + payload + "\n").encode())
                            waiting_for_reply = False   # remain in emitter mode
                        case ("CHECK_TARGET", payload):
                            ser.write(("LOOP 1:SETPT?" + "\n").encode())
                            waiting_for_reply = True   # switch to receiver mode
                        case ("CHECK_TEMPERATURE", payload):
                            ser.write(("INPUT? B" + "\n").encode())
                            waiting_for_reply = True   # switch to receiver mode
                        case ("CONTROL_ON", payload):
                            ser.write(("CONT" + "\n").encode())  
                            waiting_for_reply = False   # remain in emitter mode 
                        case ("OFF", payload):
                            ser.write(("STOP" + "\n").encode())  
                            waiting_for_reply = False   # remain in emitter mode 
                            time.sleep(2)
                            in_q.put(("CLOSE", ""))     # command itself to turn off
                        case ("CONTROL?", payload):
                            ser.write(("CONT?" + "\n").encode()) 
                            waiting_for_reply = True   # switch to receiver mode  
                        case ("SIMULATE", payload):
                            timer_T = Timer(0.5, simulate_serial, args=(state, payload))
                            timer_T.start()
                            waiting_for_reply = True   # switch to receiver mode
                        case ("CLOSE", payload):
                            is_serial_listener_running = False
                    last_payload = cmd[1]
            except queue.Empty:
                pass

        # If waiting for reply, read incoming data
        if waiting_for_reply:
            line = ser.readline().decode(errors="ignore").strip()           # In production mode, we read from the serial port
            if line:
                out_q.put(line)
                print(f"[SERIAL] message received on {ser.port}: {line}")
                waiting_for_reply = False
        time.sleep(0.5)

    ser.close()
    print("[SERIAL] Serial port closed, listener thread exited...")

    return

   
  



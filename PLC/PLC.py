import threading
import queue
import time
from serial_listener import serial_comm_thread
from tcp_listener import tcp_comm_thread
from config import PLCcfg
from experiment_state_class import ExperimentState   
from dataclasses import asdict

# ============================================================
#  Main Thread (PLC)
# ============================================================

def main():

    # Experiment state
    experiment_state = ExperimentState()  # Create an instance of the ExperimentState class

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
            match (keyword, payload):
                case (("STATUS", "iHR320_OK")):
                    print(f"[MAIN] event: iHR320 is OK")
                case (("SOMETHING_ELSE", "")):
                    print(f"[MAIN] event: something else")
                case (("INITIALISATION", str() as experiment_state_string)): 
                    print(f"[MAIN] event: initialisation with state {experiment_state_string}") 
                    # Update the experiment state from the received JSON string
                    experiment_state.deserialise(experiment_state_string)
                    print(f"[MAIN] updated experiment state: {asdict(experiment_state.experimentParameters)}")
                    cmd = "SEND"
                    msg = "INIT_CONFIRMED"
                    tcp_in.put((cmd, msg))


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

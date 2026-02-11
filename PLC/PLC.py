from re import match
import threading
import queue
import time
from serial_listener import serial_comm_thread
from tcp_listener import tcp_comm_thread
from config import PLCcfg
from experiment_state_class import ExperimentState, ExperimentStep, StepName, StepStatus   
from dataclasses import asdict
from threading import Timer

# ============================================================
#  Helper functions
# ============================================================

def reset_T_request(state):
    state["T_requested"] = False


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

    next_T = None
    state_T = {"T_requested": False}
    T_stabilisation_mins = -1
    PLC_mode = None
   
    # Main event loop
    while True:
        completed_cycle = experiment_state.experimentProgressIndex
        if completed_cycle + 1 < len(experiment_state.experimentParameters.Ts):
            next_T = experiment_state.experimentParameters.Ts[completed_cycle + 1]
        else:
            reset_PLC = True
            print(f"[MAIN] All cycles completed. Resetting PLC.")

        PLC_mode = experiment_state.experimentFlow.PLC_mode(completed_cycle)

        match PLC_mode:
            case ExperimentStep(action=StepName.TEMPERATURE, status=StepStatus.WAITING):
                if next_T :
                    ser_in.put(("GO_TO_TEMPERATURE", next_T))
                    print(f"[MAIN] Requesting next temperature {next_T} K")
                    experiment_state.experimentFlow.cycles[completed_cycle + 1].T.status = StepStatus.REQUESTED
                    state_T["T_requested"] = False
                    time.sleep(5)
            case ExperimentStep(action=StepName.TEMPERATURE, status=StepStatus.REQUESTED):
                if not state_T["T_requested"]:
                    ser_in.put(("CHECK_TEMPERATURE", ""))
                    state_T["T_requested"] = True
                    timer_T = Timer(10, reset_T_request, args=(state_T,))
                    timer_T.start()
                    print(f"[MAIN] Requesting temperature")
                else:
                    print(f"[MAIN] Waiting for temperature ")    

            case ExperimentStep(action=StepName.SPECTRUM, status=StepStatus.WAITING):
                print(f"[MAIN] Requesting spectrum step")
                ser_in.put(("ACQUIRE_SPECTRUM", ""))
            case None:
                print(f"[MAIN] No more steps to process")
        time.sleep(TIMEOUT)


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
                    experiment_state.experimentFlow.populate(len(experiment_state.experimentParameters.Ts))  # Populate the experiment flow based on the number of temperature steps
                    print(f"[MAIN] updated experiment state: {asdict(experiment_state.experimentParameters)}")
                    cmd = "SEND"
                    msg = "INIT_CONFIRMED"
                    tcp_in.put((cmd, msg))


        except queue.Empty:
            pass

        # Handle Serial events
        msg = None
        try:
            msg = ser_out.get_nowait()
            print(f"[MAIN] SERIAL event: {msg}") 
        except queue.Empty:
            pass
        if msg:
            match PLC_mode:
                case ExperimentStep(action=StepName.TEMPERATURE, status=StepStatus.WAITING):
                    try:
                        received_T = float(msg)
                        if next_T == received_T // 1 and received_T % 1 == 0:  # Check if the received temperature is an integer and matches the next_T
                            print(f"[MAIN] event: target accepted: {received_T} K")
                            experiment_state.experimentFlow.cycles[completed_cycle + 1].T.status = StepStatus.REQUESTED
                    except ValueError:
                        print(f"[MAIN] event: received non-numeric temperature value: {msg}")
                case ExperimentStep(action=StepName.TEMPERATURE, status=StepStatus.REQUESTED):
                    try: 
                        received_T = float(msg)
                        if next_T:
                            next_T_value = int(next_T)
                        else:
                            next_T_value = int(experiment_state.experimentParameters.Ts[-1])  # Default to the last temperature if next_T is None
                        if abs(next_T_value - received_T) < 0.5:  # Check if the received temperature is within 0.5 K of the next_T
                            T_stabilisation_mins += 1
                        else:
                            T_stabilisation_mins = -1  # Reset stabilisation counter if temperature is not within tolerance
                    except ValueError:
                        print(f"[MAIN] event: received non-numeric temperature value: {msg}")  
                    if T_stabilisation_mins >= 2:  # If the temperature has been stable for 3 consecutive checks (approximately 2 minutes), consider it stabilized
                            experiment_state.experimentFlow.cycles[completed_cycle + 1].T.status = StepStatus.COMPLETED
                            print(f"[MAIN] event: temperature reached")
                        
        # Periodic tasks or routing logic here
        time.sleep(TIMEOUT)


if __name__ == "__main__":
    main()

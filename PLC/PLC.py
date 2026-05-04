from re import match
import threading
import queue
import time
from serial_listener import serial_comm_thread
from tcp_listener import tcp_comm_thread
from config import ExperimentMode, PLCcfg, TCPcfg, experiment_mode, RT, temp_req_period
from experiment_state_class import ExperimentState, ExperimentStep, StepName, StepStatus, InitStep   
from dataclasses import asdict
from threading import Timer
import subprocess
import os

# ============================================================
#  Helper functions
# ============================================================

def reset_T_request(state):
    state["T_requested"] = False

def report_TC_off(tcp_in):
    print("[MAIN] event: TC disconnected")
    cmd = "SEND"
    arg = "TC_OFF"
    tcp_in.put((cmd, arg))      # Letting user know that TC is off

def report_no_exp():
    global is_experiment
    global wait_for_event
    print("[MAIN] event: iHR320 is not responsive => no experiment running")
    is_experiment = False
    wait_for_event = True

def launch_independent_server():
    # 1. Resolve paths
    plc_dir = os.path.dirname(os.path.abspath(__file__))
    server_dir = os.path.abspath(os.path.join(plc_dir, "..", "AMTCS-server"))
    venv_python = os.path.join(server_dir, "venv", "bin", "python3")

    # 2. Check if uvicorn is already running
    # 'pgrep -f' searches the full command line string
    try:
        # We check for "uvicorn" and "server:app" to be specific
        check_proc = subprocess.run(["pgrep", "-f", "uvicorn.*server:app"], capture_output=True)
        
        if check_proc.returncode == 0:
            print("[PLC] AMTCS Server is already running. Skipping launch.")
            return
    except Exception as e:
        print(f"[PLC] Error checking for existing server: {e}")

    # 3. If not running, launch it
    print(f"[PLC] Server not detected. Spawning from: {server_dir}")
    try:
        subprocess.Popen(
            [venv_python, "-m", "uvicorn", "server:app", "--host", "0.0.0.0", "--port", "8000"],
            cwd=server_dir,
            stdout=subprocess.DEVNULL, 
            stderr=subprocess.DEVNULL,
            start_new_session=True 
        )
        print("[PLC] Server successfully detached.")
    except Exception as e:
        print(f"[PLC] Error launching server: {e}")


import requests

def check_server_health(ip=TCPcfg.host, port=8000):
    url = f"http://{ip}:{port}/status"
    try:
        # We use a short timeout so the PLC doesn't hang if the server is down
        response = requests.post(url, headers={'accept': 'application/json'}, timeout=2)
        
        if response.status_code == 200:
            print(f"[PLC] Server Health Check: OK ({response.json()})")
            return True
        else:
            print(f"[PLC] Server returned error: {response.status_code}")
            return False
    except requests.exceptions.RequestException as e:
        print(f"[PLC] Server unreachable: {e}")
        return False



# ============================================================
#  Main Thread (PLC)
# ============================================================

def main():
    
    launch_independent_server()     # start server
    
    # Experiment mode (simulation or production)
    exp_mode = experiment_mode  # configure this in config.py

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
    t_ser = threading.Thread(target=serial_comm_thread, args=(ser_in, ser_out, exp_mode), daemon=True)

    t_tcp.start()
    t_ser.start()

    print("Main event loop running...")
    
    
    TIMEOUT = PLCcfg.timeout
    next_T = None
    state_T = {"T_requested": False}
    T_stabilisation_mins = -1
    PLC_mode = None
    simulated_T_TC = 300
    preExpStatus = InitStep.ID
    is_main_logic_running = True
    wait_for_event = True
    is_experiment = False
    is_experiment_suspended = False
    is_paused = False
    spectum_requested = False

    tcp_out.put(("REQUEST", "EXP_STATUS"))                  # Task to itself: Request the experiment status

    # Main event loop
    while is_main_logic_running:

        if is_experiment:
            completed_cycle = experiment_state.experimentProgressIndex
            if completed_cycle + 1 < experiment_state.experimentLength:
                if not is_paused:
                    wait_for_event = False
                is_experiment = True
                next_T = experiment_state.experimentParameters.Ts[completed_cycle + 1]
            else:
                wait_for_event = True
                is_experiment = False
                print(f"[MAIN] All cycles completed. Resetting PLC.")
                print(f"[MAIN] No more steps to process")
                tcp_in.put(("SEND", "EXPERIMENT_FINISHED"))

            if not is_experiment_suspended:
                PLC_mode = experiment_state.experimentFlow.PLC_mode(completed_cycle)
            else:
                PLC_mode = None

            match PLC_mode:
                case ExperimentStep(action=StepName.TEMPERATURE, status=StepStatus.WAITING):
                    if next_T:
                        ser_in.put(("GO_TO_TEMPERATURE", next_T))
                        state_T["T_requested"] = False
                        print(f"[MAIN] Requesting next temperature {next_T} K")
                        time.sleep(5)
                        ser_in.put(("CHECK_TARGET", ""))
                case ExperimentStep(action=StepName.TEMPERATURE, status=StepStatus.REQUESTED):
                    if not state_T["T_requested"]:
                        ser_in.put(("CHECK_TEMPERATURE", ""))
                        state_T["T_requested"] = True
                        timer_T = Timer(temp_req_period, reset_T_request, args=(state_T,))
                        timer_T.start()
                        print(f"[MAIN] Requesting temperature")
                        if exp_mode == ExperimentMode.SIMULATION:
                            target_T = int(next_T) if next_T else int(experiment_state.experimentParameters.Ts[-1]) 
                            simulated_T_TC += (target_T - simulated_T_TC)*0.5   # Two times closer to the target temperature every time
                            ser_in.put(("SIMULATE", str(simulated_T_TC)))       # Trigger the temperature simulation
                    else:
                        print(f"[MAIN] Waiting for temperature ")    
                case ExperimentStep(action=StepName.TEMPERATURE, status=StepStatus.COMPLETED):
                    print(f"[MAIN] event: temperature reached")
                    experiment_state.experimentFlow.cycles[completed_cycle + 1].S.status = StepStatus.WAITING
                case ExperimentStep(action=StepName.SPECTRUM, status=StepStatus.WAITING):
                    print(f"[MAIN] Requesting spectrum step")
                    if not spectum_requested:
                        tcp_in.put(("SEND", "ACQUIRE_SPECTRUM " + next_T))
                        spectum_requested = True
                case ExperimentStep(action=StepName.SPECTRUM, status=StepStatus.REQUESTED):
                    print(f"[MAIN] Waiting for spectrum being measured")
                case ExperimentStep(action=StepName.SPECTRUM, status=StepStatus.COMPLETED):
                    print(f"[MAIN] Spectrum acquired, moving to next cycle")
                    experiment_state.experimentProgressIndex += 1   
                    continue 
            time.sleep(TIMEOUT)


        # Handle TCP events
        try:
            if wait_for_event:
                (keyword, payload) = tcp_out.get()
            else:
                (keyword, payload) = tcp_out.get_nowait()
            print(f"[MAIN] TCP event: {keyword}: {payload}")
            match (keyword, payload):
                case (("iHR320", "PING")):
                    print(f"[MAIN] event: fresh instance of iHR320 is detected")
                    if is_experiment:
                        is_paused = True
                    is_experiment_suspended = True
                    wait_for_event = False              # Turn of waiting while in initialisation
                    cmd = "SEND"
                    if check_server_health():
                        arg = "SERVER_ONLINE"
                    else: 
                        arg = "SERVER_OFFLINE"   
                    tcp_in.put((cmd, arg))
                case (("STATUS", "iHR320_ERROR")):
                    if is_experiment:
                        wait_for_event = True 
                        is_paused = True
                    print(f"[MAIN] event: UI App stayed silent for too long. Check it.")
                case (("IHR320", "TC_STATUS")):
                    print("[MAIN] event: TC status requested...")
                    cmd = "TC_STATUS"
                    arg = ""
                    ser_in.put((cmd, arg))
                    timer_TC = Timer(10, report_TC_off, args=(tcp_in,))
                    timer_TC.start()                    # After 10 sec, report TC status "not responsive"
                case (("REQUEST", "EXP_STATUS")):
                    print("[MAIN] event: Experiment status requested...")
                    cmd = "SEND"
                    arg = "REQUEST EXP_STATUS?"
                    tcp_in.put((cmd, arg))
                    timer_exp_status = Timer(10, report_no_exp)
                    timer_exp_status.start()            # After 10 sec, report TC status "not responsive"
                case (("IHR320", "EXP_STATE?")):
                    print("[MAIN] event: Experiment state requested by UI App...") 
                    if is_experiment:
                        cmd = "SEND"
                        arg = "EXP: " + experiment_state.serialise()
                        tcp_in.put((cmd, arg))
                        experiment_state.experimentFlow.cycles[completed_cycle + 1].T.status = StepStatus.WAITING
                    else:
                        cmd = "SEND"
                        arg = "EXP_NONE"
                        tcp_in.put((cmd, arg))
                case (("IHR320", "USER_CANCEL")):
                    print("[MAIN] event: Experiment has been cancelled by user...") 
                    # Artificially ending experiment
                    experiment_state.experimentProgressIndex = experiment_state.experimentLength
                case (("IHR320", "USER_PAUSE")):
                    print("[MAIN] event: Experiment has been paused by user...")
                    wait_for_event = True
                    is_paused = True
                case (("IHR320", "USER_CONTINUE")):
                    print("[MAIN] event: Experiment has been resumed by user...")
                    wait_for_event = False
                    is_paused = False
                case (("IHR320", "REQUESTED_OFF")):
                    print("[MAIN] event: requested to stop PLC")
                    cmd = "OFF"
                    arg = ""
                    ser_in.put((cmd, arg))
                    tcp_in.put((cmd, arg))
                    time.sleep(5)                       # After 5 sec, set while condition false
                    is_main_logic_running = False
                case (("EXP_STATUS", "NOT_STARTED")):
                    print("[MAIN] event: Experiment not yet started...\n")
                    timer_exp_status.cancel()
                    is_experiment = False
                    wait_for_event = True
                case (("EXP_STATUS", "RUNNING")):
                    print("[MAIN] event: Experiment already running... Requesting experiment state")
                    timer_exp_status.cancel()
                    cmd = "SEND"
                    arg = "REQUEST EXP_STATE?"
                    tcp_in.put((cmd, arg))
                case (("AFFIRMATIVE", "SPECTRUM_REQUESTED")):
                    experiment_state.experimentFlow.cycles[completed_cycle + 1].S.status = StepStatus.REQUESTED
                    print(f"[MAIN] event: spectrum requested")
                case (("ERROR", "SPECTRUM_REQUESTED")):
                    experiment_state.experimentFlow.cycles[completed_cycle + 1].S.status = StepStatus.WAITING
                    print(f"[MAIN] event: spectrum request failed. Check UI App.")
                    is_paused = True
                case (("REPORT", "SPECTRUM_ACQUIRED")):
                    experiment_state.experimentFlow.cycles[completed_cycle + 1].S.status = StepStatus.COMPLETED
                    print(f"[MAIN] event: spectrum acquired")    
                case (("INITIALISATION", str() as experiment_state_string)): 
                    print(f"[MAIN] event: initialisation with state {experiment_state_string}") 
                    try:
                        # Update the experiment state from the received JSON string
                        experiment_state.deserialise(experiment_state_string)
                        # Populate the experiment flow based on the exp length and current progress index
                        experiment_state.experimentFlow.populate(experiment_state.experimentLength, experiment_state.experimentProgressIndex)
                        print(f"[MAIN] updated experiment state: {asdict(experiment_state.experimentParameters)}")
                        cmd = "SEND"
                        arg = "EXP_CONFIRMED"
                        tcp_in.put((cmd, arg))
                        is_experiment = True
                        is_paused = False
                    except ValueError as e:
                        # Inform iHR320 app of error
                        cmd = "SEND"
                        arg = "EXP_ERROR " + str(e)
                        tcp_in.put((cmd, arg))


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
#-----------------------TC initialisation-----------------------------------------------------------------------------------
                case None:                                  # Serial communication logic before experiment started
                    match preExpStatus:
                        case InitStep.ID:                   # Checking TC model after "IDN?" ASCII command
                            if msg == "Cryocon Model 32, Rev 6.08H":
                                timer_TC.cancel()
                                print("TC alive")
                                cmd = "SEND"
                                arg = "TC_OK"
                                tcp_in.put((cmd, arg))      # If model confirmed, letting iHR320(C++) know that TC is alive
                                preExpStatus = InitStep.CURRENT_T
                                cmd = "CHECK_TEMPERATURE"
                                arg = ""
                                ser_in.put((cmd, arg))      # Requesting current temperature
                            else:
                                report_TC_off(tcp_in,)       # Letting user know that TC is off
                        case InitStep.CURRENT_T:            # Checking current T 
                            try: 
                                received_T = float(msg[0:5])
                                print(f"[MAIN] event: received temperature: {received_T} K")
                                cmd = "SEND"
                                arg = f"T= {received_T}"
                                tcp_in.put((cmd, arg))      # Sending current T to iHR320
                                preExpStatus = InitStep.SETPT_T
                                cmd = "GO_TO_TEMPERATURE"   
                                arg = f"{int(received_T)}"
                                ser_in.put((cmd, arg))      # Set target T to value of current T  

                                timer_T = Timer(2, ser_in.put, args=(("CHECK_TARGET", ""),))
                                timer_T.start()             # After 2 sec, request current target T
                            except ValueError:
                                cmd = "CHECK_TEMPERATURE"
                                arg = ""
                                ser_in.put((cmd, arg))      # Re-requesting current T    
                                print(f"[MAIN] event: No expected response received: {msg}. Retrying...")
                        case InitStep.SETPT_T:
                            try: 
                                target_T = int(float(msg[0:4]))
                                print(f"[MAIN] event: received target temperature: {received_T} K")
                                if (abs(target_T - received_T) < 1):
                                    cmd = "CONTROL_ON"
                                    arg = ""
                                    ser_in.put((cmd, arg))  # Requesting control "ON" as it is save to do so
                                    preExpStatus = InitStep.CONTROL
                                    timer_T = Timer(2, ser_in.put, args=(("CONTROL?", ""),))
                                    timer_T.start()         # After 2 sec, request status of control
                                else:                       # If target T != current T, repeating previous step
                                    cmd = "CHECK_TEMPERATURE"
                                    arg = ""
                                    ser_in.put((cmd, arg))  # Requesting current temperature
                                    preExpStatus = InitStep.CURRENT_T
                            except ValueError:
                                cmd = "CHECK_TARGET"
                                arg = ""
                                ser_in.put((cmd, arg))      # Re-requesting current target T    
                                print(f"[MAIN] event: No expected response received: {msg}. Retrying...")  
                        case InitStep.CONTROL:
                            if (msg == "ON"):
                                cmd = "SEND"
                                arg = "TC_READY"
                                tcp_in.put((cmd, arg))      # Updating TC status to "ready" for iHR320
                                preExpStatus = InitStep.ID
                                wait_for_event = True       # Turn on waiting upon end of initialisation
                                is_experiment_suspended = False
                            else:
                                cmd = "CONTROL_ON"
                                arg = ""
                                ser_in.put((cmd, arg))
                                timer_T = Timer(2, ser_in.put, args=(("CONTROL?", ""),))
                                timer_T.start()             # After 2 sec, request status of control

#-----------------------Experiment flow-----------------------------------------------------------------------------------
                case ExperimentStep(action=StepName.TEMPERATURE, status=StepStatus.WAITING):
                    try:
                        spectum_requested = False       # Flag spectrum not requested at the start of a new cycle
                        T_stabilisation_mins = -1       # Reset T stabilisation
                        received_T = int(float(msg[0:4]))           
                        if next_T == str(received_T):   # Check if the received temperature matches the next_T
                            print(f"[MAIN] event: target accepted: {received_T} K")
                            cmd = "SEND"
                            arg = "TARGET_T= " + str(received_T)
                            tcp_in.put((cmd, arg))      # Inform iHR320 about new T target
                            experiment_state.experimentFlow.cycles[completed_cycle + 1].T.status = StepStatus.REQUESTED
                    except ValueError:
                        print(f"[MAIN] event: received non-numeric temperature value: {msg}")
                case ExperimentStep(action=StepName.TEMPERATURE, status=StepStatus.REQUESTED):
                    try: 
                        received_T = float(msg[0:5])
                        print(f"[MAIN] event: received temperature: {received_T} K")
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
        time.sleep(TIMEOUT)


if __name__ == "__main__":
    main()

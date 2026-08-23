import signal
import sys
import threading
from fastapi import FastAPI, Header, HTTPException, Depends, Request
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import socket
from .config import TCPcfg, PLC_SCRIPT_NAME, PLC_SCRIPT_PATH, PYTHON
import psutil
import subprocess
from enum import Enum
import os
from datetime import datetime, timezone



#*************************HELPERS**************************#

class ExpStatus(Enum):
    NOT_STARTED = -1
    UNKNOWN = 0
    RUNNING = 1
    PAUSED = 2
    FINISHED = 3
    
class Log(BaseModel):
    timestamp: str
    text: str
    
class Task(BaseModel):
    task: str

class ExperimentDetails(BaseModel):
    status: int
    length: int
    progress: int

class StringParam(BaseModel):
    value: str   

# Load API key from file
#def load_api_key():
#    base = os.path.dirname(os.path.realpath(__file__))
#    key_path = os.path.abspath(os.path.join(base, "..", ".api_key"))
#    with open(key_path, "r") as f:
#        return f.read().strip()
    
# TCP communication with PLC (fire-and-forget)
def notify_plc(message: str):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(timeout)  # to prevent hanging
            s.connect((PLC_HOST, PLC_PORT))
            s.sendall(message.encode("utf-8"))
            return True
    except Exception as e:
        print(f"PLC notification failed: {e}")
        return False
        
# Start PLC script
def start_plc():
    subprocess.Popen(
        [PYTHON, PLC_SCRIPT_PATH + PLC_SCRIPT_NAME],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        cwd=PLC_SCRIPT_PATH
    )
        
# Restart PLC by running the script (for future release)
def restart_plc():
    for proc in psutil.process_iter(['pid', 'cmdline']):
        cmd = proc.info['cmdline']
        if cmd and PLC_SCRIPT_NAME in cmd:
            proc.kill()                                             # Kill existing PLC.py processes
    start_plc()                                                     # ...then start a fresh instance


# Save logs to a timestamped folder and return the file path (for future release)
def save_logs_to_folder():
    folder_timestamp = datetime.now().strftime("%Y%m%d %H:%M")      # Build timestamped folder 
    folder_path = f"./__logs__/{folder_timestamp}"
    os.makedirs(folder_path, exist_ok=True)                         # Create directory if it doesn't exist
    file_path = f"{folder_path}/logs.txt"                           # Full path to logs.txt
    with open(file_path, "w", encoding="utf-8") as f:
        for log in logs:
            f.write(f"{log.timestamp}: {log.text}\n")               # Write logs line by line in logs.txt
    return file_path

# Shut down server
def shutdown_server():
    os.kill(os.getpid(), signal.SIGTERM)
        
#*************************END OF HELPERS**************************#
       
PLC_HOST = TCPcfg.host
PLC_PORT = TCPcfg.SEND_PORT
timeout = TCPcfg.timeout

exp_status = ExpStatus.UNKNOWN
exp_length = 0
exp_progress = -1

logs = []
task = None 

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


API_KEY = "PL1234"


# Security dependency helper to verify API key in request headers 
async def verify_api_key(client_api_key: str = Header(None)):
    if client_api_key != API_KEY:
        raise HTTPException(status_code=401, detail="Invalid or missing API key")

# PLC sets server access code
@app.post("/new_access_code", dependencies=[Depends(verify_api_key)])
def set_access_code(new_access_code: StringParam):
    global API_KEY
    API_KEY = new_access_code.value
    return {"status": "Accepted"}

# List of logs and current experiment status reported to the mobile app to display on the dashboard
@app.get("/", dependencies=[Depends(verify_api_key)])
def root():
    global exp_status, exp_length, exp_progress
    return {
        "logs": logs,
        "experiment_status": exp_status.value,
        "experiment_length": exp_length,
        "experiment_progress": exp_progress
    }

# PLC reports start of experiment
@app.post("/experiment_start", dependencies=[Depends(verify_api_key)])
def experiment_start(exp_details: ExperimentDetails):
    global exp_status, exp_length, exp_progress
    match exp_details.status:
        case -1: exp_status = ExpStatus.NOT_STARTED
        case 0: exp_status = ExpStatus.UNKNOWN
        case 1: exp_status = ExpStatus.RUNNING
        case 2: exp_status = ExpStatus.PAUSED
        case 3: exp_status = ExpStatus.FINISHED         
    exp_length = exp_details.length
    exp_progress = exp_details.progress
    log = Log(
        timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%S"),
        text = "Experiment has been (re)started."
    ) 
    logs.append(log)
    return {"status": "Accepted"}

# PLC reports a log message to be added to the server's log list
@app.post("/add_log", dependencies=[Depends(verify_api_key)])
async def add_log(request: Request):
    global logs, exp_status, exp_length, exp_progress
    payload = await request.json()
    # Extract Log fields
    log = Log(
        timestamp=payload["log"]["timestamp"],
        text=payload["log"]["text"]
    )
    # Extract ExperimentDetails fields
    exp_details = ExperimentDetails(
        status=payload["exp_details"]["status"],
        length=payload["exp_details"]["length"],
        progress=payload["exp_details"]["progress"]
    )
    # Update experiment state
    match exp_details.status:
        case -1: exp_status = ExpStatus.NOT_STARTED
        case 0:  exp_status = ExpStatus.UNKNOWN
        case 1:  exp_status = ExpStatus.RUNNING
        case 2:  exp_status = ExpStatus.PAUSED
        case 3:  exp_status = ExpStatus.FINISHED
    exp_length = exp_details.length
    exp_progress = exp_details.progress
    logs.append(log)
    return {"status": "Accepted"}

# Mob app requests to pause the experiment
@app.post("/experiment/status_request/pause", dependencies=[Depends(verify_api_key)])
def pause_experiment():
    global exp_status, task
    task = Task(task="__PAUSE__")
    return { "status": "Suspending experiment has been requested." }

@app.post("/experiment/status_report/not_started", dependencies=[Depends(verify_api_key)])
def experiment_not_started():
    global exp_status, logs
    log = Log(
        timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%S"),
        text = "Experiment is ready to be started."
    ) 
    logs.append(log)
    exp_status = ExpStatus.NOT_STARTED
    return {"status": "Accepted"}
    
# PLC reports UI paused the experiment    
@app.post("/experiment/status_report/pause", dependencies=[Depends(verify_api_key)])
def experiment_paused():
    global exp_status, logs
    log = Log(
        timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%S"),
        text = "Experiment is paused by user"
    ) 
    logs.append(log)
    exp_status = ExpStatus.PAUSED
    return {"status": "Accepted"}

# Mob app requests to resume the experiment
@app.post("/experiment/status_request/resume", dependencies=[Depends(verify_api_key)])
def resume_experiment():
    global exp_status, task
    task = Task(task="__RESUME__")
    return { "status": "Resuming experiment has been requested." }

# PLC reports UI started/resumed the experiment    
@app.post("/experiment/status_report/running", dependencies=[Depends(verify_api_key)])
def experiment_running():
    global exp_status, logs
    log = Log(
        timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%S"),
        text = "Experiment is resumed by user"
    ) 
    logs.append(log)
    exp_status = ExpStatus.RUNNING
    return {"status": "Accepted"}

# PLC reports that the experiment is finished    
@app.post("/experiment/status_report/finished", dependencies=[Depends(verify_api_key)])
def experiment_finished():
    global exp_status, logs
    log = Log(
        timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%S"),
        text = "Experiment is finished"
    ) 
    logs.append(log)
    exp_status = ExpStatus.FINISHED
    return {"status": "Accepted"}

# Mob app requests to cancel the experiment
@app.post("/experiment/status_request/cancel", dependencies=[Depends(verify_api_key)])
def cancel_experiment():
    global exp_status, task
    task = Task(task="__CANCEL__")
    return { "status": "Cancellation of experiment has been requested" }

# PLC requests current task from the server (pause/resume/cancel), which is set by the mobile app/browser app
@app.get("/current_task", dependencies=[Depends(verify_api_key)])
def get_current_task():
    global task
    current_task = task.task if task else None
    task = None                            # Clear the task after it's been reported to the PLC
    return { "task": current_task }

# PLC reports UI cancelled the experiment
@app.post("/experiment/status_report/cancel", dependencies=[Depends(verify_api_key)])
def experiment_cancelled():
    global exp_status, exp_length, exp_progress, logs
    log = Log(
        timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%S"),
        text = "Experiment has been cancelled by user."
    ) 
    logs.append(log)
    exp_status = ExpStatus.UNKNOWN
    exp_length = 0
    exp_progress = -1
    return {"status": "Accepted"}    
    
# Mob app/PLC requests to restart PLC    
@app.post("/plc/restart", dependencies=[Depends(verify_api_key)])   # for future release
def plc_restart():
    restart_plc()
    return {"status": "PLC has been restarted; check status in a few minutes"}

# PLC requests to save logs to a timestamped folder and return the file path
@app.post("/save_logs", dependencies=[Depends(verify_api_key)])
def save_logs():
    return {
        "status": "Logs saved to folder", 
        "file_path": save_logs_to_folder()
    }
    
# PLC requests server status
@app.get("/status")
def server_status():
    return { "status": "OK" }

# Mob app requests to shut down the server
@app.post("/shutdown", dependencies=[Depends(verify_api_key)])   
def shutdown(): 
    threading.Thread(target=shutdown_server).start()                # Shutdown in a separate thread to allow a meaningful response 
    return {"status": "Server is shutting down..."}                 # to be sent before the server actually stops

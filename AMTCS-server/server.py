import signal
import sys
import threading
from fastapi import FastAPI, Header, HTTPException, Depends
from pydantic import BaseModel
import socket
from config import TCPcfg, PLC_SCRIPT_NAME, PLC_SCRIPT_PATH, PYTHON
import psutil
import subprocess
from enum import Enum
import os
from datetime import datetime


#*************************HELPERS**************************#

class ExpStatus(Enum):
    UNKNOWN = 0
    RUNNING = 1
    PAUSED = 2
    
class Log(BaseModel):
    timestamp: str
    text: str

# Load API key from file
def load_api_key():
    with open(".api_key", "r") as f:
        return f.read().strip()

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

logs = [] 

app = FastAPI()

API_KEY = load_api_key()


# Security dependency helper to verify API key in request headers 
async def verify_api_key(client_api_key: str = Header(None)):
    if client_api_key != API_KEY:
        raise HTTPException(status_code=401, detail="Invalid or missing API key")

# List of logs and current experiment status reported to the mobile app to display on the dashboard
@app.get("/", dependencies=[Depends(verify_api_key)])
def root():
    return {
        "logs": logs,
        "experiment_status": exp_status.value
    }

# PLC reports a log message to be added to the server's log list
@app.post("/add_log", dependencies=[Depends(verify_api_key)])
def add_log(log: Log):
    logs.append(log)
    return {"status": "Accepted"}

# Mob app request to update the experiment status, which is forwarded to the PLC
@app.post("/experiment/status_request/update", dependencies=[Depends(verify_api_key)])
def update_experiment_status():
    if notify_plc("EXP_STATUS"):
        return {"status": "Experiment status update requested from PLC"}
    else: 
        return {"error": "PLC is not reachable"}

# Mob app requests to pause the experiment, which is forwarded to the PLC
@app.post("/experiment/status_request/pause", dependencies=[Depends(verify_api_key)])
def pause_experiment():
    global exp_status
    if notify_plc("EXP_SUSPEND"):
        exp_status = ExpStatus.PAUSED
        return {
            "status": "PLC notified of experiment to be paused",
            "experiment_status": exp_status.value
        }
    else: 
        return {"error": "PLC is not reachable"}
    
# PLC reports UI paused the experiment    
@app.post("/experiment/status_report/pause", dependencies=[Depends(verify_api_key)])
def experiment_paused():
    global exp_status
    exp_status = ExpStatus.PAUSED
    return {"status": "Accepted"}

# Mob app requests to resume the experiment, which is forwarded to the PLC
@app.post("/experiment/status_request/resume", dependencies=[Depends(verify_api_key)])
def resume_experiment():
    global exp_status
    if notify_plc("EXP_RESUME"):
        exp_status = ExpStatus.RUNNING
        return {
            "status": "PLC notified of experiment to be resumed",
            "experiment_status": exp_status.value
        }
    else: 
        return {"error": "PLC is not reachable"}

# PLC reports UI started/resumed the experiment    
@app.post("/experiment/status_report/running", dependencies=[Depends(verify_api_key)])
def experiment_running():
    global exp_status
    exp_status = ExpStatus.RUNNING
    return {"status": "Accepted"}

# Mob app requests to cancel the experiment, which is forwarded to the PLC
@app.post("/experiment/status_request/cancel", dependencies=[Depends(verify_api_key)])
def cancel_experiment():
    global exp_status
    if notify_plc("EXP_CANCEL"): 
        exp_status = ExpStatus.UNKNOWN
        return {
            "status": "PLC notified of experiment to be cancelled",
            "experiment_status": exp_status.value
        }
    else: 
        return {"error": "PLC is not reachable"}

# PLC reports UI cancelled the experiment
@app.post("/experiment/status_report/cancel", dependencies=[Depends(verify_api_key)])
def experiment_cancelled():
    global exp_status
    exp_status = ExpStatus.UNKNOWN
    return {"status": "Accepted"}    
    
# Mob app requests to restart PLC    
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

# Mob app requests to shut down the server
@app.post("/shutdown", dependencies=[Depends(verify_api_key)])   
def shutdown(): 
    threading.Thread(target=shutdown_server).start()                # Shutdown in a separate thread to allow a meaningful response 
    return {"status": "Server is shutting down..."}                 # to be sent before the server actually stops

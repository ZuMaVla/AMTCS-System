from dataclasses import dataclass

@dataclass
class TCPcfg:
    host = "192.168.50.1"
    SEND_PORT   = 5050
    timeout = 1

PLC_SCRIPT_NAME = "PLC.py"
PLC_SCRIPT_PATH = "../PLC/"
PYTHON = "/usr/bin/python3"
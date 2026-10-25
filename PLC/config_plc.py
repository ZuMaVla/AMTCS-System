from dataclasses import dataclass
from enum import Enum

@dataclass
class TCPcfg:
    remote_host = "192.168.50.11"
    host = "192.168.50.1"
    LISTEN_PORT = 5050
    SEND_PORT   = 5051
    timeout = 1

@dataclass
class PLCcfg:
    timeout = 2

class ExperimentMode(Enum):
    SIMULATION = 0
    PRODUCTION = 1

RT = "294"                      # Room temperature in K
temp_req_period = 30            # How often to check temperature when in experiment
#experiment_mode = ExperimentMode.SIMULATION   # Change to ExperimentMode.PRODUCTION for actual production mode 
experiment_mode = ExperimentMode.PRODUCTION   # Change to ExperimentMode.PRODUCTION for actual production mode 

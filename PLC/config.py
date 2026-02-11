from dataclasses import dataclass
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
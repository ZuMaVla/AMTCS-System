from dataclasses import dataclass
@dataclass
class TCPcfg:
    remote_host = "192.168.50.11"
    host = "192.168.50.1"
    port = 5050
    timeout = 0.2

@dataclass
class PLCcfg:
    timeout = 0.1
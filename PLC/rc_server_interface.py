import queue
import time
#from PLC import get_current_task
#from PLC import RCServerStopFlag

def rc_server_poller(tcp_in: queue.Queue, tcp_out: queue.Queue, stop_flag, get_current_task):
    print("[RC-SERVER Interface] : started")
    while not stop_flag.value:
        task = get_current_task()
        if task:
            match task:
                case "__PAUSE__":
                    print("[RC-SERVER] User requested to PAUSE experiment")
                    tcp_out.put(("IHR320", "USER_PAUSE"))
                    tcp_in.put(("SEND", "CONFIRM_PAUSE_CONTINUE"))

                case "__RESUME__":
                    print("[RC-SERVER] User requested to RESUME experiment")
                    tcp_out.put(("IHR320", "USER_CONTINUE"))
                    tcp_in.put(("SEND", "CONFIRM_PAUSE_CONTINUE"))

                case "__CANCEL__":
                    print("[RC-SERVER] User requested to CANCEL experiment")
                    tcp_out.put(("IHR320", "USER_CANCEL"))
                    tcp_in.put(("SEND", "CONFIRM_CANCEL"))

        time.sleep(2)
    print("[RC-SERVER Interface] : turned off")    

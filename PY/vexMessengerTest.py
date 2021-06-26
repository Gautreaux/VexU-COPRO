import time
from random import randbytes
from .vexMessenger import VexMessenger
from . import v_messenger

def bytesTest():
    TOTAL_BYTES = 1*(2**20) # 8 MB
    BYTES_PER_MESSAGE = 96

    rounds = (TOTAL_BYTES // BYTES_PER_MESSAGE) + 1

    nextReport = 0
    reportFreq = .005

    k = ""
    has_rcv = False

    def echoAckCallback(msg):
        nonlocal k
        nonlocal has_rcv

        has_rcv = True

        if msg.data != k:
            print(f"Sent: {k}")
            print(f"Recv: {msg.data}")
            assert(msg.data == k)

    v_messenger.setEchoAckCallback(echoAckCallback)

    startTime = time.time()

    for i in range(rounds):
        k = randbytes(BYTES_PER_MESSAGE)
        msg = VexMessenger._Message(k, 5)
        v_messenger._send_message(msg)

        while has_rcv is False:
            v_messenger.readIfAvailable()
        has_rcv = False

        p = i / rounds
        if p > nextReport:
            nextReport += reportFreq
            elapsed = time.time() - startTime
            print(f"{round(p*BYTES_PER_MESSAGE,2)}% complete, {BYTES_PER_MESSAGE*i} bytes, {round(elapsed, 2)} sec, {round(BYTES_PER_MESSAGE*i/(elapsed * 1024), 4)} Kbps")
        
    elapsed = time.time() - startTime
    print(f"Sent {round((rounds * BYTES_PER_MESSAGE)/(1024), 2)} Kilobytes without error in {round(elapsed,2)} sec, approx speed: {round((rounds*BYTES_PER_MESSAGE)/(1024*elapsed), 4)} Kbps")
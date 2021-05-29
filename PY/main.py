import itertools
import threading
from time import sleep
from .vexSerial import ECHO_SIG, VexSerialTeardown, VexSerialWaitStreamSync, setVexSerialCallback, sendMessage, VexSerialDefaultCallback, VexSerialWaitForConnection 
from .vexSerial import setEchoCallback, _sendControlMessage, v_ser
from .vexSerial import vexSerialTest

def main():
    setVexSerialCallback(VexSerialDefaultCallback)
    # mainLogic()
    VexSerialWaitForConnection()
    for _ in range(40):
        sleep(.05)
    v_ser.write(b"\x03\x00\x04p")
    for _ in range(40):
        sleep(.05)
    v_ser.write(b"\x00\x04p")
    for _ in range(40):
        sleep(.05)
    print("Starting teardown")
    VexSerialTeardown()
    print("Nominal exit")
    exit(0)
    

    try:
        # vexSerialTest.testRandomDatagram(128)

        def tempEchoCallback(bb : bytes):
            print(f"Echo response: {bb}")
        setEchoCallback(tempEchoCallback)
        # m = bytes(itertools.chain([*ECHO_SIG, 5], b"45678"))
        # print(f"Sending: {m}")
        # _sendControlMessage(m)
        # VexSerialWaitStreamSync(b'\x69')

        testCases = [
            "m", "n", "o", "p", "pppp", "op", "po", "\x04p"
        ]

        for case in testCases:
            print(f"Starting {case}")
            m = bytes(itertools.chain([*ECHO_SIG, len(case)], bytes(case, encoding="ascii")))
            _sendControlMessage(m)
            sleep(.05)
            VexSerialWaitStreamSync(b'\x00')
        for case in testCases:
            print(f"Starting {case}")
            m = bytes(itertools.chain([*ECHO_SIG, len(case)], bytes(case, encoding="ascii")))
            _sendControlMessage(m)
            sleep(.05)
            VexSerialWaitStreamSync(b'\x44')
        for case in testCases:
            print(f"Starting {case}")
            m = bytes(itertools.chain([*ECHO_SIG, len(case)], bytes(case, encoding="ascii")))
            _sendControlMessage(m)
            sleep(.05)
            VexSerialWaitStreamSync(b'\x04')
        for case in testCases:
            print(f"Starting {case}")
            m = bytes(itertools.chain([*ECHO_SIG, len(case)], bytes(case, encoding="ascii")))
            _sendControlMessage(m)
            sleep(.05)
            VexSerialWaitStreamSync(b'\x70')
        print("Starting terardown")
        VexSerialTeardown()
        print("Nominal exit")
        exit(0)

        for i in range(256):
            print(f"Starting {i}")
            m = bytes(itertools.chain([*ECHO_SIG, 1], [i]))
            print(f"Sending: {m}")
            _sendControlMessage(m)
            sleep(.05)
            VexSerialWaitStreamSync(bytes([i]))
        
        # rr = list(itertools.chain(range(256), range(256)))
        # block_len = 10
        # for i in range(256):
        #     print(f"Starting block {i}")
        #     m = bytes(itertools.chain([*ECHO_SIG, block_len], rr[i:i+block_len]))
        #     _sendControlMessage(m)
        #     VexSerialWaitStreamSync(bytes([i+128]))

        for i in range(10):
            print(f"Starting test: {i}")
            vexSerialTest.testRandomDatagram(128)
            VexSerialWaitStreamSync(bytes([i]))
        
        vexSerialTest.testRandomDatagram(2**10)
        VexSerialWaitStreamSync(b'\x69')

        while True:
            i = input("Enter message: ")
            sendMessage(bytes(i, encoding="ascii"))
    except KeyboardInterrupt:
        VexSerialTeardown()

if __name__ == "__main__":
    # this file was run directly
    print("Running directly (no init) (this <probably> wont matter?)")
    main()
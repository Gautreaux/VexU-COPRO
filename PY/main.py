import itertools
from .vexSerial import ECHO_SIG, VexSerialTeardown, setVexSerialCallback, sendMessage, VexSerialDefaultCallback, VexSerialWaitForConnection 
from .vexSerial import _setEchoCallback, _sendControlMessage
from .vexSerial import vexSerialTest

def main():
    setVexSerialCallback(VexSerialDefaultCallback)
    # mainLogic()
    VexSerialWaitForConnection()
    try:
        vexSerialTest.testRandomDatagram(128)

        # def tempEchoCallback(bb : bytes):
        #     print(f"Echo response: {bb}")
        # _setEchoCallback(tempEchoCallback)
        # m = bytes(itertools.chain([*ECHO_SIG, 5], b"45678"))
        # print(f"Sending: {m}")
        # _sendControlMessage(m)

        while True:
            i = input("Enter message: ")
            sendMessage(bytes(i, encoding="ascii"))
    except KeyboardInterrupt:
        VexSerialTeardown()

if __name__ == "__main__":
    # this file was run directly
    print("Running directly (no init) (this <probably> wont matter?)")
    main()
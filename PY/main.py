from PY.vexSerial.vexSerialTest import testAllDatagrams
from .vexSerial.vexSerial import VexSerial
from .vexSerial import getVexComPort 

def tempCallback(_ : VexSerial, b : bytes) -> None:
    print(b, flush=True)

def main():
    # testAllDatagrams()
    # exit()

    myPort = getVexComPort()
    print(f"Resolved VEX Device port to: {myPort}")

    vser = VexSerial(myPort, tempCallback)

    if bool(vser):
        print("Serial port opened successfully")
    else:
        print("Serial port failed to open")
        exit(0)

    try:
        while True:
            i = input("Enter message to send: ")
            i = i + '\n'
            vser.sendData(i.encode("ascii"))
    except KeyboardInterrupt:
        pass

    print("Stopping vser")
    vser._stopRecv()

if __name__ == "__main__":
    # this file was run directly
    print("Running directly (no init) (this <probably> wont matter?)")
    main()
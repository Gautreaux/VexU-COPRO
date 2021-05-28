from .vexSerial import setVexSerialCallback, sendMessage, VexSerialDefaultCallback, waitForConnection



def main():
    setVexSerialCallback(VexSerialDefaultCallback)
    # mainLogic()
    waitForConnection()
    while True:
        i = input("Enter message: ")
        sendMessage(bytes(i, encoding="ascii"))

    # testAllDatagrams()
    exit()

    myPort = getVexComPort()
    print(f"Resolved VEX Device port to: {myPort}")

    v_ser = VexSerial(myPort, tempCallback)

    if bool(v_ser):
        print("Serial port opened successfully")
    else:
        print("Serial port failed to open")
        exit(0)

    try:
        while True:
            i = input("Enter message to send: ")
            i = i + '\n'
            v_ser.sendData(i.encode("ascii"))
    except KeyboardInterrupt:
        pass

    print("Stopping v_ser")
    v_ser._stopRecv()

if __name__ == "__main__":
    # this file was run directly
    print("Running directly (no init) (this <probably> wont matter?)")
    main()
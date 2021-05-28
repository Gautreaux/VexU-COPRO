from .vexSerial import VexSerialTeardown, setVexSerialCallback, sendMessage, VexSerialDefaultCallback, VexSerialWaitForConnection



def main():
    setVexSerialCallback(VexSerialDefaultCallback)
    # mainLogic()
    VexSerialWaitForConnection()
    try:
        while True:
            i = input("Enter message: ")
            sendMessage(bytes(i, encoding="ascii"))
    except KeyboardInterrupt:
        VexSerialTeardown()

if __name__ == "__main__":
    # this file was run directly
    print("Running directly (no init) (this <probably> wont matter?)")
    main()
from .vexMessenger import v_messenger
from .vexMessenger.vexMessengerTest import bytesTest
from .vexController import vexAction

def main():

    v_messenger.connect()
    print("Messenger connected")
    # time.sleep(2)
    v_messenger.sendMessage(b"Starting Bytes Test")
    # time.sleep(2)
    bytesTest()
    v_messenger.sendMessage(b"Finished Bytes Test")
    v_messenger.disconnect()
    print("Messenger disconnected")

if __name__ == "__main__":
    # this file was run directly
    print("Running directly (no init) (this <probably> wont matter?)")
    main()
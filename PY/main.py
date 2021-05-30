from .vexSerial import v_ser

from time import sleep

def main():

    v_ser.sendMessage(b"unicorn")
    msg = v_ser.receiveMessage()
    print(f"Received message: {msg}")
    pass

if __name__ == "__main__":
    # this file was run directly
    print("Running directly (no init) (this <probably> wont matter?)")
    main()
from .vexSerial import v_ser
# from .vexSerial.vexSerialTest import wordTest, bytesTest
from .vexMessenger import v_messenger
from .CV import cv_main

import time

def main():

    v_messenger.connect()
    print("Messenger connected")
    time.sleep(2)
    # v_ser.sendMessage(b"\x09\x00\x00\x00apple")
    v_messenger.sendMessage(b"Apples Rock")
    time.sleep(2)
    v_messenger.disconnect()
    print("Messenger disconnected")

    # cv_main()

    # wordTest()
    # print("Word test done")
    # bytesTest()
    # print()
    # print("Byte test done")
    # print("Test concluded successfully")

if __name__ == "__main__":
    # this file was run directly
    print("Running directly (no init) (this <probably> wont matter?)")
    main()
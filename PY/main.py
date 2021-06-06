# from .vexSerial import v_ser
# from .vexSerial.vexSerialTest import wordTest, bytesTest
from PY.mouseOdometry.mouseOdometryUtil import determineMiceByPath
from .vexMessenger import v_messenger
# from .vexMessenger.vexMessengerTest import bytesTest
# from .CV import cv_main
# from PY.mouseOdometry.mouseOdometryUtil import determineValidMice, listMice
# from .mouseOdometry.mouseOdometry import calibrateOdom, getCurrentOdomPosition, launchOdom
from .vexController import vexAction


import time

def main():

    print(determineMiceByPath(["1.2", "1.3"]))
    exit(0)

    v_messenger.connect()
    # v_messenger.sendMessage(b"\x0B")
    # vexAction.VEX_resetIMU() # takes too long and is non-blocking in pros
    vexAction.VEX_startRotation(True)
    time.sleep(3)
    # v_messenger.sendMessage(b"\x00")
    vexAction.VEX_stop()
    print(f"IMU value: {vexAction.VEX_readIMU()}")
    v_messenger.disconnect()
    exit(0)
    
    # print("Test mouse odometry")

    # l = listMice()
    # print("All mice: ", end="")
    # print(l)
    # p = determineValidMice()
    # print("Relevant Mice: ", end="")
    # print(p)

    # print("Overriding mice for testing:")
    # p = ["/dev/input/mouse2", "/dev/input/mouse0"]

    # if len(p) == 0:
    #     print("No relevant mic could be found")
    #     return
    # elif len(p) == 1:
    #     print("Only one relevant mice could be found")
    #     return
    # elif len(p) != 2:
    #     print(f"Expected two relevant mice, found {len(p)} instead")
    #     return

    # launchOdom(*p)
    # calibrateOdom()

    # time.sleep(500)

    # while(True):
    #     print(getCurrentOdomPosition())

    # v_messenger.connect()
    # print("Messenger connected")
    # # time.sleep(2)
    # v_messenger.sendMessage(b"Starting Bytes Test")
    # # time.sleep(2)
    # bytesTest()
    # v_messenger.sendMessage(b"Finished Bytes Test")
    # v_messenger.disconnect()
    # print("Messenger disconnected")

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
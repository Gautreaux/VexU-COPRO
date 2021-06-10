# from .vexSerial import v_ser
# from .vexSerial.vexSerialTest import wordTest, bytesTest
from PY.mouseOdometry.mouseOdometry import getCurrentDeltas, getCurrentOdomPosition, launchOdomLoop, launchReadLoop, resetCurrentDeltas, resolveDValue, shutdownOdom, LEFT_MOUSE_USB_PATH, RIGHT_MOUSE_USB_PATH
from PY.mouseOdometry.mouseOdometryUtil import determineDFromDeltasRotation, determineMiceByPath, determineTranslationFromDelta
from .vexMessenger import v_messenger
# from .vexMessenger.vexMessengerTest import bytesTest
# from .CV import cv_main
# from PY.mouseOdometry.mouseOdometryUtil import determineValidMice, listMice
# from .mouseOdometry.mouseOdometry import calibrateOdom, getCurrentOdomPosition, launchOdom
from .vexController import vexAction

import os
import select
import time
import threading
from typing import Final

def main():
    import os
    print(os.getcwd())

    with open("sampledata.csv", 'r') as inFile:
        i = iter(inFile)
        headers = next(i)
        for line in i:
            l = list(map(lambda x: float(x.strip()), line.strip().split(",")))
            print(l)
            print(f"  {determineDFromDeltasRotation(l[:4], l[4])}")

    exit(0)

    RESOLVED_D = 8.2

    micePaths = determineMiceByPath([LEFT_MOUSE_USB_PATH, RIGHT_MOUSE_USB_PATH])
    print(micePaths)
    launchReadLoop(*micePaths)

    resetCurrentDeltas()

    try:
        time.sleep(5000)
    except KeyboardInterrupt:
        pass

    print(f"Deltas: {getCurrentDeltas()}")

    exit(0)
    launchOdomLoop(RESOLVED_D)
    
    v_messenger.connect()
    imu_start = vexAction.VEX_readIMU()
    
    while True:
        now_imu = vexAction.VEX_readIMU()
        delta_imu = now_imu - imu_start
        _, _, delta_odom = getCurrentOdomPosition()
        print(f"IMU: {delta_imu}, ODOM: {delta_odom}")
        print(f"  Difference: {delta_imu - delta_odom}")
        time.sleep(1)

    vexAction.VEX_stop()
    v_messenger.disconnect()
    shutdownOdom()

    # resolveDValue()
    # exit(0)
    
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
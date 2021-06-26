import multiprocessing
if multiprocessing.current_process().name == "MainProcess":
    # print(f"Importing messenger/serial/action: {multiprocessing.parent_process()} {multiprocessing.current_process().name}")
    import vexAction
    from vexMessenger import *
    from vexSerial import *
import vexCV
from vexGPIO import *

import time

global v_ser
global v_message
v_ser = None
v_messenger = None

def main():
    global v_ser
    global v_message
    v_ser = None
    v_messenger = None

    power_led, connection_led, goal_led, ball_led = getLEDs()

    def update_leds():
        power_led.update()
        connection_led.update()
        goal_led.update()
        ball_led.update()

    update_leds()

    power_led.setColor(RGB.GREEN)
    power_led.setOp(Pattern.ON)

    goal_led.setOp(Pattern.OFF)
    ball_led.setOp(Pattern.OFF)

    connection_led.setColor(RGB.RED)
    connection_led.setOp(Pattern.HALF_BLINK)

    while v_ser is None:
        try:
            v_ser = VexSerial()
        except NoSerialConnectionException:
            update_leds()

    print("Serial Link established")

    connection_led.setColor((1,1,0))
    update_leds()

    v_messenger = VexMessenger(v_ser)

    print("Starting messenger connection...")
    while v_messenger.isConnected is False:
        v_messenger.try_connect(1)
        update_leds()
    print("Messenger Connected")

    connection_led.setOp(Pattern.ON)
    connection_led.setColor(RGB.GREEN)
    update_leds()

    cv_args = {
        "showAnnotated": False,
        "sharpen": False,
    }

    cameras = [None,None]

    # cameras[0] = (
    #     "../Adhoc/CV/dev/15.1.avi",
    #     False
    # )

    # cameras[1] = (
    #     "../Adhoc/CV/dev/lowCam_15_ball.avi",
    #     False
    # )

    cameras[0] = (0, True)
    cameras[1] = (1, True)

    g_p, b_p, q = vexCV.cv_mp(cv_args, cameras)

    try:
        while True:
            # just sleep forever while cameras do things
            # s = v_messenger.readDataMessageBlocking()
            isGoal, data = q.get()
            # print(f"outbound message: {isGoal} {data}")
            if isGoal:
                vexAction.VEX_sendGoalTarget(data)
                print(f"goal_sending{data}")
            else:
                vexAction.VEX_sendBallTarget(data)
                print(f"ball_sending{data}")
    except KeyboardInterrupt:
        print("Stopping processes...")
        if g_p:
            g_p.kill()
        if b_p:
            b_p.kill()
        print("Killed child processes...")

    exit(0)

    vexCV.cvSetup((0, True))

    frame_counter = 0
    start_time = time.time()

    REPORT_INTERVAL_FRAMES = 64


    while True:
        # msg = input("Enter message: ")
        # vexAction.VEX_text(msg)
        vexCV.cvStep(True)

        frame_counter += 1

        if(frame_counter >= REPORT_INTERVAL_FRAMES):
            now_time = time.time()
            elapsed = round((now_time - start_time) * 1000)

            print(f"{REPORT_INTERVAL_FRAMES} frames processed in {elapsed} ms ({round((1000 * REPORT_INTERVAL_FRAMES) / (elapsed))} fps)")

            start_time = now_time
            frame_counter = 0

    v_messenger.disconnect()

    # v_messenger.connect()
    # print("Messenger connected")
    # # time.sleep(2)
    # v_messenger.sendMessage(b"Starting Bytes Test")
    # # time.sleep(2)
    # bytesTest()
    # v_messenger.sendMessage(b"Finished Bytes Test")
    # v_messenger.disconnect()
    # print("Messenger disconnected")

if __name__ == "__main__":
    try:
        main()
    finally:
        GPIO.cleanup()
import multiprocessing
if multiprocessing.current_process().name == "MainProcess":
    print(f"Importing messenger/serial/action: {multiprocessing.parent_process()} {multiprocessing.current_process().name}")

    from vexMessenger import v_messenger
    from vexController import vexAction
from vexCV import vexCV

import time

def main():

    print("Stating messenger connection...")
    v_messenger.connect()
    print("Messenger Connected")

    cv_args = {
        "showAnnotated": True,
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
    cameras[1] = (2, True)

    g_p, b_p, q = vexCV.cv_mp(cv_args, cameras)

    try:
        while True:
            # just sleep forever while cameras do things
            # s = v_messenger.readDataMessageBlocking()
            isGoal, data = q.get()
            # print(f"outbound message: {isGoal} {data}")
            if isGoal:
                vexAction.VEX_sendGoalTarget(data)
            else:
                vexAction.VEX_sendBallTarget(data)
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
    main()
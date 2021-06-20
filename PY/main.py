from .vexMessenger import v_messenger
from .vexMessenger.vexMessengerTest import bytesTest
from .vexController import vexAction
from .vexCV import vexCV

import time

def main():

    print("Stating messenger connection...")
    v_messenger.connect()

    vexCV.cvSetup((1, True))

    frame_counter = 0
    start_time = time.time()

    REPORT_INTERVAL_FRAMES = 64


    while True:
        # msg = input("Enter message: ")
        # vexAction.VEX_text(msg)
        vexCV.cvStep()

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
    # this file was run directly
    print("Running directly (no init) (this <probably> wont matter?)")
    main()
# some windows compatibility bs
if __name__ == "__main__":
    import sys
    import os
    import time

    search_path = os.path.abspath(__file__).rpartition('\\')[0] + "/../../PY/"
    sys.path.insert(1, search_path)
    from vexCV import vexCV
    import cv2 as cv
    import numpy as np

    # print(f"CWD: {os.getcwd()}")

    cameras = [
        (os.path.abspath(__file__).rpartition('\\')[0] + "/dev/15.1.avi", False),
        (os.path.abspath(__file__).rpartition('\\')[0] + "/dev/lowCam_15_ball.avi", False),
    ]

    args = {
        "showAnnotated" : True,
        "sharpen" : False,
    }

    vexCV.cv_mp(args, cameras)

    print("Vex CV, MP started, main sleeping")

    try:
        while True:
            time.sleep(30)
            print("MAIN")
    except KeyboardInterrupt:
        pass
    
    exit(0)

    FRAMES_TO_SKIP = 420
    FRAMES_TO_SKIP = 50

    vexCV.cvSetup(
        [
            (os.path.abspath(__file__).rpartition('\\')[0] + "/dev/15.1.avi", False),
            (os.path.abspath(__file__).rpartition('\\')[0] + "/dev/lowCam_15_ball.avi", False),
            # (os.path.abspath(__file__).rpartition('\\')[0] + "/dev/swirlTestVid8.mp4", False),
            # (os.path.abspath(__file__).rpartition('\\')[0] + "/dev/swirlTestVid12.mp4", False),

        ],
        FRAMES_TO_SKIP)
    # vexCV.cvSetup([(0, True), (1, True)])
    start_time = time.time()
    REPORT_INTERVAL_FRAMES = 64
    frame_counter = 0


    while True:
        vexCV.cvStep(True)

        frame_counter += 1

        if(frame_counter >= REPORT_INTERVAL_FRAMES):
            now_time = time.time()
            elapsed = round((now_time - start_time) * 1000)

            print(f"{REPORT_INTERVAL_FRAMES} frames processed in {elapsed} ms ({round((1000 * REPORT_INTERVAL_FRAMES) / (elapsed))} fps)")

            start_time = now_time
            frame_counter = 0
        
        # print("-------------------------------------------")
        # while True:
        #     if cv.waitKey(10) & 0xFF == ord('q'):
        #         break
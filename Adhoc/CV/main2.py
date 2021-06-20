import sys
import os
import time


search_path = os.path.abspath(__file__).rpartition('\\')[0] + "/../../PY/"
sys.path.insert(1, search_path)
from vexCV import vexCV
import cv2 as cv
import numpy as np

# print(f"CWD: {os.getcwd()}")

FRAMES_TO_SKIP = 420

vexCV.cvSetup((os.path.abspath(__file__).rpartition('\\')[0] + "/dev/15.1.avi", False), FRAMES_TO_SKIP)

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
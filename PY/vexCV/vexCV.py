import cv2 as cv
import numpy as np
from . import cv_template, cv_template_h, cv_template_w, cv_capture
from ..vexController import vexAction

def scaleGenerator(step: int = .05):
    assert(step > 0)
    i = 1.0
    while i > 0:
        yield i
        i -= step

def getScaledImage(frame, scale):
    assert(scale > 0)
    assert(scale <= 1)
    if scale == 1:
        return frame
    targetDims = (
        int(frame.shape[1] * scale),
        int(frame.shape[0] * scale)
    )
    return cv.resize(frame, targetDims)

def connectCamera():
    global cv_capture
    if cv_capture is not None:
        return
    
    print("Starting video connection")
    cv_capture = cv.VideoCapture(1, cv.CAP_DSHOW)
    print("Camera Connected")

# get and return the best guess goal position
def getBestGoalPosition(frame, show_annotated : bool = False):
    if show_annotated:
        annotated = frame.copy()

    frame_gray = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)

    # list of rectangles matching the template, scaled wrt the main image
    template_rectangles = []

    for scale in scaleGenerator():
        frame_gray_scaled = getScaledImage(frame_gray, scale)

        scaled_w, scaled_h = frame_gray_scaled.shape

        if scaled_w < cv_template_w or scaled_h < cv_template_h:
            # image has been down scaled too small
            break

        # scaled_edge = cv.Canny(cv.GaussianBlur(frame_gray_scaled, (3,3), 0), 50, 150)
        scaled_edge = cv.Canny(frame_gray_scaled, 50, 150)
        res = cv.matchTemplate(scaled_edge, cv_template, cv.TM_CCOEFF_NORMED)

        threshold = .60
        loc = np.where(res >= threshold)
        for pt in zip(*loc[::-1]):
            # print(f"Found template match at pt {pt} scale {scale}")
            template_rectangles.append(((int(pt[0]/scale), int(pt[1]/scale)), (int((pt[0] + cv_template_h)/scale), int((pt[1] + cv_template_w)/scale))))

    if not template_rectangles:
        # no matches found
        if show_annotated:
            cv.namedWindow("annotated", cv.WINDOW_NORMAL)
            cv.imshow("annotated", annotated)
            cv.waitKey(10)

        return None
    
    template_rectangles.sort(key = lambda x : ((x[0][0] + x[1][0]) / 2))
    groups = []
    maxX = -1

    for up_left, low_right in template_rectangles:
        x = (up_left[0] + low_right[0]) / 2
        y = (up_left[1] + low_right[1]) / 2
        if x > maxX:
            groups.append([])
        groups[-1].append((x, y))
        maxX = max(maxX, low_right[0])

    biggest_group = max(groups, key=len)

    sum_x = 0
    sum_y = 0

    for x,y in biggest_group:
        sum_x += x
        sum_y += y

    c_x = int(sum_x / len(biggest_group))
    c_y = int(sum_y / len(biggest_group))

    # draw the template matches
    if show_annotated:
        for up_left, low_right in template_rectangles:
            cv.rectangle(annotated, up_left, low_right, (0,30, 128), thickness=2)
            

    # try and resolve distance
    largest_rect = max(template_rectangles, key=(lambda x : x[1][0] - x[0][0]))

    frame_hsv = cv.cvtColor(frame, cv.COLOR_BGR2HSV)
    frame_blurry_af = cv.GaussianBlur(frame_hsv, (15,15), 0)
    ff_mask = np.zeros((frame_blurry_af.shape[0] + 2, frame_blurry_af.shape[1] + 2, 1), np.uint8)
    some_int, colored, mask, bounding  = cv.floodFill(frame_hsv, ff_mask, (c_x, c_y + 20), (0,0, 255), loDiff=(25,50,50), upDiff=(25,50,50), flags=(4 | cv.FLOODFILL_FIXED_RANGE))

    x0, y0, w, h = bounding
    
    if show_annotated:
        cv.rectangle(annotated, (x0, y0), (x0 + w, y0 + h), (0, 255, 0), 2)
        cv.circle(annotated, (c_x, c_y), 15, (0, 0, 255), thickness=-1)
        cv.namedWindow("annotated", cv.WINDOW_NORMAL)
        cv.imshow("annotated", annotated)
        cv.waitKey(10)

    if w < 5 or h < 5:
        w = 0
        h = 0

    return (c_x, c_y, w, h)


def cvStep():
    connectCamera()
    r, f = cv_capture.read()

    if not r:
        # no new frame
        return
    group_center = getBestGoalPosition(f, True)
    if group_center:
        vexAction.VEX_sendGoalTarget(group_center)
        print(group_center)
    else:
        vexAction.VEX_sendGoalTarget((0,0,0,0))
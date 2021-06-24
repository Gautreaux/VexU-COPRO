import cv2 as cv
import multiprocessing
import numpy as np
import time

REPORT_INTERVAL_FRAMES = 64

from . import cv_template, cv_template_h, cv_template_w

# how many goal guess points are needed for a consensus
CONSENSUS_THRESHOLD = 5

# what % of image should be same black to indicate blocked image
BLOCKED_CAM_THRESHOLD = 0.3

# how many frames can miss before no-goal established
MAX_NON_FRAME = 5

# store info about the filter
filter_datastore = [0]*16

#enums to make things easier
FILTER_LAST_RETURNED = 0
FILTER_LAST_TTL = 1

BALL_COLOR_BLUE = 0
BALL_COLOR_RED = 1
BALL_COLOR_ANY = -1

# show an image and block
#   do not put into a non-debug loop
def showImageBlock(images):
    for name, frame in images:
        cv.namedWindow(name, cv.WINDOW_NORMAL)
        cv.imshow(name, frame)

    while True:
        if cv.waitKey(10) & 0xFF == ord('q'):
            break

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

# returns the capture object
def connectCamera(camera_details):
    cap = None

    print("Starting video connection")
    camera, isWebcam = camera_details
    if isWebcam:
        cap = cv.VideoCapture(camera, cv.CAP_DSHOW)
        # print(cap.get(cv.CAP_PROP_EXPOSURE))
        cap.set(cv.CAP_PROP_EXPOSURE, -4) 
    else:
        cap = cv.VideoCapture(camera)
    print("Camera Connected")
    return cap

# magic sharpening function
def unsharp_mask(image, kernel_size=(5, 5), sigma=1.0, amount=1.0, threshold=0):
    """Return a sharpened version of the image, using an unsharp mask."""
    blurred = cv.GaussianBlur(image, kernel_size, sigma)
    sharpened = float(amount + 1) * image - float(amount) * blurred
    sharpened = np.maximum(sharpened, np.zeros(sharpened.shape))
    sharpened = np.minimum(sharpened, 255 * np.ones(sharpened.shape))
    sharpened = sharpened.round().astype(np.uint8)
    if threshold > 0:
        low_contrast_mask = np.absolute(image - blurred) < threshold
        np.copyto(sharpened, image, where=low_contrast_mask)
    return sharpened

# yields the initial image, then generates sharpened variants of the image
def sharpenGenerator(frame, run_sharpen):
    yield frame

    if not run_sharpen:
        return

    # do a simple gaussian sharpen
    f_g = cv.GaussianBlur(frame, (0,0), 3)
    yield cv.addWeighted(frame, 1.5, f_g, -0.5, 0, f_g)

    # do a very, very aggressive sharpen
    yield unsharp_mask(frame, kernel_size=(11,11), amount=5)

    # these sometimes resolve things, but mostly slow things down
    #   they dont get things that the other 3 get

    # do a simple kernel shapen
    #   this generally, doesn't find things, but sometimes
    # k = np.array([[-1,-1,-1], [-1,9,-1], [-1,-1,-1]])
    # yield cv.filter2D(frame, -1, k)

    # yield unsharp_mask(frame)

# get a list of possible goal positions in the image
#   returns a list of rectangles in the image
def getPossibleGoalPosition(frame):
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

    return template_rectangles

# partion the list template rectangles
#   int a list of lists of rectangles
def partionGuesses(template_rectangles):
    template_rectangles.sort(key = lambda x : ((x[0][0] + x[1][0]) / 2))
    groups = []
    maxX = -1

    for up_left, low_right in template_rectangles:
        x = (up_left[0] + low_right[0]) / 2
        y = (up_left[1] + low_right[1]) / 2
        if x > maxX:
            groups.append([])
        groups[-1].append(((up_left, low_right, x, y)))
        maxX = max(maxX, low_right[0])
    
    return groups

# get and return the best guess goal position
def getBestGoalPosition(frame, frame_to_annotate = None, run_sharpen = True):
    template_rectangles = []
    biggest_group = []

    for f in sharpenGenerator(frame, run_sharpen):
        new_rect = getPossibleGoalPosition(f)
        if not new_rect:
            # no new possibilities
            continue
        
        template_rectangles.extend(new_rect)

        groups = partionGuesses(template_rectangles)

        biggest_group = max(groups, key=len)

        if len(biggest_group) > CONSENSUS_THRESHOLD:
            break

    if not biggest_group:
        # no matches found
        if frame_to_annotate is not None:
            pass

        return None
    
    sum_x = 0
    sum_y = 0

    for _,_,x,y in biggest_group:
        sum_x += x
        sum_y += y

    c_x = int(sum_x / len(biggest_group))
    c_y = int(sum_y / len(biggest_group))
            
    # draw the template matches
    if frame_to_annotate is not None:
        for up_left, low_right, _,_, in biggest_group:
            cv.rectangle(frame_to_annotate, up_left, low_right, (0,30, 128), thickness=2)

    # try and resolve distance
    largest_rect = max(biggest_group, key=(lambda x : x[1][0] - x[0][0]))
    w = (largest_rect[1][0] - largest_rect[0][0])
    h = (largest_rect[1][1] - largest_rect[0][1])
    
    if frame_to_annotate is not None:
        cv.circle(frame_to_annotate, (c_x, c_y), 15, (0, 0, 255), thickness=-1)

    return (c_x, c_y, w, h)

# NOT WORKING
#   hough circles didnt work
#   need some hsv thresholds
def isBallInImage(frame_hsv) -> str:
    return None

def isCameraBlocked(frame_gray) -> bool:
    hist_array = cv.calcHist(frame_gray, [0], None, [8], [0,256])
    s = sum(hist_array)

    # width, height = frame_gray.shape
    # pixels_total = width * height

    discriminator = s * BLOCKED_CAM_THRESHOLD

    return (hist_array[0] > discriminator)

# manages the filtering of goal, hiding drops, ect.
def goalFilter(pre_filter):
    global filter_datastore

    if pre_filter:
        # some goal was resolved
        #   is this consistent with previous guess?
        #       we dont check this

        filter_datastore[FILTER_LAST_RETURNED] = pre_filter
        filter_datastore[FILTER_LAST_TTL] = MAX_NON_FRAME

        return pre_filter
    else:
        # no goal was detected
        # TODO - logic about allowing some period of misses before 
        # what about switching to another resolver if we are close to a goal
        #   returning a no goal

        if filter_datastore[FILTER_LAST_TTL]:
            filter_datastore[FILTER_LAST_TTL] -= 1

            # return nothing (let old value linger)
            #   this currently makes things worse
            # return None

        # return that no goal could be resolved
        return (0,0,0,0)

# returns capture
def cvSetup(camera_details, frames_to_skip = 0):
    cap = connectCamera(camera_details)

    while frames_to_skip > 0:
        r,f = cap.read()
        frames_to_skip -= 1

    return cap

# block until the next frame occurs
#   return the new frame
def getNextFrameBlocking(capture):
    r,f = capture.read()

    while not r:
        time.sleep(.05)
        r,f = capture.read()
    return f

def findGoals(f, frame_to_annotate, q, run_sharpen):
    group_center = getBestGoalPosition(f, frame_to_annotate, run_sharpen)

    if group_center is None:
        # f_hsv = cv.cvtColor(f, cv.COLOR_BGR2HSV)

        # ball_color = isBallInImage(f_hsv)
        # if ball_color:
        #     print(f"Ball {ball_color} in image")
        #     return
        f_gray = cv.cvtColor(f, cv.COLOR_BGR2GRAY)
        if isCameraBlocked(f_gray):
            print("GOAL CAMERA BLOCKED")
            return

    filtered = goalFilter(group_center)
    # print(f"{group_center} --> {filtered}")
    if filtered:
        q.put((True, filtered))
    
    if frame_to_annotate is not None:
        if filter_datastore[FILTER_LAST_TTL] != MAX_NON_FRAME and filter_datastore[FILTER_LAST_TTL] and filter_datastore[FILTER_LAST_RETURNED]:
            # draw the circle, but stale
            cv.circle(frame_to_annotate, filter_datastore[FILTER_LAST_RETURNED][:2], 15, (0,0,255), -1)
            cv.circle(frame_to_annotate, filter_datastore[FILTER_LAST_RETURNED][:2], 12, (255 * filter_datastore[FILTER_LAST_TTL] / (MAX_NON_FRAME),0,0), -1)

def findBalls(f, frame_to_annotate, q, color):
    low_hsv = cv.cvtColor(f, cv.COLOR_BGR2HSV)

    colors_found = []
    color_contours = []

    if color == BALL_COLOR_BLUE or color == BALL_COLOR_ANY:
        blue_h_vals = cv.inRange(low_hsv, np.array([100,100,0]), np.array([140,255,255]))
        blue_cont, _ = cv.findContours(blue_h_vals, cv.RETR_LIST, cv.CHAIN_APPROX_NONE)
        
        colors_found.append(BALL_COLOR_BLUE)
        color_contours.append(blue_cont)
        # cv.drawContours(frame_to_annotate, blue_cont, -1, (225, 128, 0), 3)

    if color == BALL_COLOR_RED or color == BALL_COLOR_ANY:
        red1 = cv.inRange(low_hsv, np.array([0,100,0]), np.array([20,255,255]))
        red2 = cv.inRange(low_hsv, np.array([160,100,0]), np.array([180,255,255]))
        red_h_vals = np.bitwise_or(red1, red2)
        red_cont, _ = cv.findContours(red_h_vals, cv.RETR_LIST, cv.CHAIN_APPROX_NONE)

        colors_found.append(BALL_COLOR_RED)
        color_contours.append(red_cont)
        # cv.drawContours(frame_to_annotate, red_cont, -1, (0, 128, 255), 3)

    ballLocations = []

    for color_n, contours in zip(colors_found, color_contours):
        draw_color = ((255,1828,0) if color_n == BALL_COLOR_RED else (0,128,255))
        
        for c in contours:
            # cv.drawContours(frame_to_annotate, [c], 0, (0, 255, 0), 3)
            # continue

            actualArea = cv.contourArea(c)
            if(actualArea < 4096):
                # too small ( < 32x32 px)
                continue

            x,y,w,h = cv.boundingRect(c)

            # height_ratio = (w/h)
            # if abs(height_ratio - 1) > .5:
            #     # wrong shape
            #     continue

            # pixels in contour to rectangle size
            #   effectively, holey-ness and regularness check
            fullness_ratio = actualArea/(w*h)
            if fullness_ratio < .50:
                # too sparse
                continue

            c_x = int(x+w/2)
            c_y = int(y+h/2)
            c_r = int((w + h) / 4)

            ballLocations.append((c_x, c_y, c_r, color_n))

            if frame_to_annotate is not None:
                #  cv.drawContours(frame_to_annotate, [c], 0, draw_color, 3)
                # print(ballLocations[-1])
                cv.circle(frame_to_annotate, ballLocations[-1][:2], ballLocations[-1][2], draw_color, 2)

    if q:
        for ball in ballLocations:
            q.put((False, ball))
        return None
    return ballLocations

def findBestBall(f, frame_to_annotate, q, color):
    ballLocations = findBalls(f, frame_to_annotate, None, color)

    # print(f"Ball Locations: {ballLocations}")

    if ballLocations:
        best_ball = max(ballLocations, key=(lambda x : x[2]))
        # print(f"BEST BALL {best_ball}")
        q.put((False, best_ball))

        if frame_to_annotate is not None:
            cv.circle(frame_to_annotate, best_ball[:2], best_ball[2], (0,255,0), 3)
    else:
        # print("NO BALLS")
        q.put((False, (0,0,0,0)))

    time.sleep(.5)

# entry point for multi-processing
def mp_entry_common(args, isGoals, camera_details, q):
    cap = cvSetup(camera_details)

    show_annotated = args['showAnnotated']
    run_sharpen = args['sharpen']

    short_name = "GOALS" if isGoals else "BALLS"

    frame_counter = 0
    start_time = time.time()

    while True:
        frame_counter += 1

        f = getNextFrameBlocking(cap)

        if show_annotated:
            frame_to_annotate = f.copy()
        else:
            frame_to_annotate = None

        try:
            if isGoals:
                findGoals(f, frame_to_annotate, q, run_sharpen)
            else:
                findBestBall(f, frame_to_annotate, q, BALL_COLOR_RED)
        finally:
            if show_annotated:
                cv.namedWindow(f"{short_name}_annotated", cv.WINDOW_NORMAL)
                cv.imshow(f"{short_name}_annotated", frame_to_annotate)
                cv.waitKey(1)

            if(frame_counter >= REPORT_INTERVAL_FRAMES):
                now_time = time.time()
                elapsed = round((now_time - start_time) * 1000)

                print(f"{short_name}: ({round((1000 * REPORT_INTERVAL_FRAMES) / (elapsed))} fps): {REPORT_INTERVAL_FRAMES} frames processed in {elapsed} ms ")

                start_time = now_time
                frame_counter = 0

# run the cv loop via multiprocessing
#   returns handles to the processes
def cv_mp(args, cameras):
    print("CV_MP")
    
    q = multiprocessing.Queue()

    if not cameras:
        print("Error, no cameras provided")

    if cameras[0]:
        goals_process = multiprocessing.Process(target=mp_entry_common, args=(args, True, cameras[0], q), daemon=True)
        goals_process.start()
    else:
        goals_process = None

    if cameras[1]:
        balls_process = multiprocessing.Process(target=mp_entry_common, args=(args, False, cameras[1], q), daemon=True)
        balls_process.start()
    else:
        balls_process = None

    return (goals_process, balls_process, q)

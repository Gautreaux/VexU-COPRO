import cv2 as cv
import numpy as np
from enum import Enum, unique
from . import cv_template, cv_template_h, cv_template_w, cv_capture
try:
    from ..vexController import vexAction
except ImportError:
    print("Error importing vex action.")
    print("  If you are running adhoc this is expected.")
    print("  If not, this is a problem.")

    # create a proxy for vexAction
    class vexAction:
        @classmethod
        def VEX_sendGoalTarget(_, target):
            print(f"Proxy send: {target}")
            pass

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

def connectCamera(camera_details):
    global cv_capture
    if cv_capture is not None:
        return
    
    print("Starting video connection")
    if camera_details[1]:
        cv_capture = cv.VideoCapture(camera_details[0], cv.CAP_DSHOW)
    else:
        cv_capture = cv.VideoCapture(camera_details[0])
    print("Camera Connected")


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
def sharpenGenerator(frame):
    yield frame

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
#   int a list of lists
def partionGuesses(template_rectangles):
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
    
    return groups

# get and return the best guess goal position
def getBestGoalPosition(frame, frame_to_annotate = None):
    template_rectangles = []
    biggest_group = []

    for f in sharpenGenerator(frame):
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

    for x,y in biggest_group:
        sum_x += x
        sum_y += y

    c_x = int(sum_x / len(biggest_group))
    c_y = int(sum_y / len(biggest_group))

    # draw the template matches
    if frame_to_annotate is not None:
        for up_left, low_right in template_rectangles:
            cv.rectangle(frame_to_annotate, up_left, low_right, (0,30, 128), thickness=2)
            

    # try and resolve distance
    #largest_rect = max(template_rectangles, key=(lambda x : x[1][0] - x[0][0]))

    frame_hsv = cv.cvtColor(frame, cv.COLOR_BGR2HSV)
    frame_blurry_af = cv.GaussianBlur(frame_hsv, (15,15), 0)
    ff_mask = np.zeros((frame_blurry_af.shape[0] + 2, frame_blurry_af.shape[1] + 2, 1), np.uint8)
    some_int, colored, mask, bounding  = cv.floodFill(frame_hsv, ff_mask, (c_x, c_y + 20), (0,0, 255), loDiff=(25,50,50), upDiff=(25,50,50), flags=(4 | cv.FLOODFILL_FIXED_RANGE))

    x0, y0, w, h = bounding
    
    if frame_to_annotate is not None:
        cv.rectangle(frame_to_annotate, (x0, y0), (x0 + w, y0 + h), (0, 255, 0), 2)
        cv.circle(frame_to_annotate, (c_x, c_y), 15, (0, 0, 255), thickness=-1)

    if w < 5 or h < 5:
        w = 0
        h = 0

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

            # return nothing
            return None

        # return that no goal could be resolved
        return (0,0,0,0)

def cvSetup(camera_path, frames_to_skip = 0):
    connectCamera(camera_path)

    while frames_to_skip > 0:
        r,f = cv_capture.read()
        frames_to_skip -= 1

def cvStep(show_annotated : bool = False):
    r, f = cv_capture.read()

    if show_annotated:
        frame_to_annotate = f.copy()
    else:
        frame_to_annotate = None

    try:
        if not r:
            # no new frame
            return
        group_center = getBestGoalPosition(f, frame_to_annotate)

        if group_center is None:
            # f_hsv = cv.cvtColor(f, cv.COLOR_BGR2HSV)

            # ball_color = isBallInImage(f_hsv)
            # if ball_color:
            #     print(f"Ball {ball_color} in image")
            #     return
            f_gray = cv.cvtColor(f, cv.COLOR_BGR2GRAY)
            if isCameraBlocked(f_gray):
                print("CAMERA BLOCKED")
                return

        filtered = goalFilter(group_center)
        print(f"{group_center} --> {filtered}")
        if filtered:
            vexAction.VEX_sendGoalTarget(filtered)
    finally:
        if show_annotated:
            if filter_datastore[FILTER_LAST_TTL] != MAX_NON_FRAME and filter_datastore[FILTER_LAST_RETURNED]:
                # draw the circle, but stale
                cv.circle(frame_to_annotate, filter_datastore[FILTER_LAST_RETURNED][:2], 15, (0,0,255), -1)
                cv.circle(frame_to_annotate, filter_datastore[FILTER_LAST_RETURNED][:2], 12, (255 * filter_datastore[FILTER_LAST_TTL] / (MAX_NON_FRAME),0,0), -1)

            cv.namedWindow("annotated", cv.WINDOW_NORMAL)
            cv.imshow("annotated", frame_to_annotate)
            cv.waitKey(1)
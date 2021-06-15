import cv2 as cv
import numpy as np
import math
import time

SHOW_NO_BLOCK = 1
SHOW_BLOCK = 2

def openVideoCapture():
    capture = cv.VideoCapture(0)
    
    if not capture.isOpened():
        print("Camera could not be opened")
        exit()
    
    print("Camera connected")

    while True:
        ret, frame = capture.read()
        
        cv.imshow('video', frame)

        if cv.waitKey(1) & 0xFF == ord('q'):
            break
    
    capture.release()

def recordVideo():
    capture = cv.VideoCapture(0)
    out = cv.VideoWriter('out_vid.avi', cv.VideoWriter_fourcc('M', 'J', 'P', 'G'), 10, (int(capture.get(3)), int(capture.get(4))))

    while True:
        r,f = capture.read()

        if not r:
            break

        out.write(f)

        cv.imshow('frame', f)

        if cv.waitKey(1) & 0xFF == ord('q'):
            break   

def imageHSVSliders(f):
    # Create a window
    cv.namedWindow('image')

    def nothing(x):
        pass

    # Create trackbars for color change
    # Hue is from 0-179 for Opencv
    cv.createTrackbar('HMin', 'image', 0, 179, nothing)
    cv.createTrackbar('SMin', 'image', 0, 255, nothing)
    cv.createTrackbar('VMin', 'image', 0, 255, nothing)
    cv.createTrackbar('HMax', 'image', 0, 179, nothing)
    cv.createTrackbar('SMax', 'image', 0, 255, nothing)
    cv.createTrackbar('VMax', 'image', 0, 255, nothing)

    # Set default value for Max HSV trackbars
    cv.setTrackbarPos('HMax', 'image', 179)
    cv.setTrackbarPos('SMax', 'image', 255)
    cv.setTrackbarPos('VMax', 'image', 255)

    # Initialize HSV min/max values
    hMin = sMin = vMin = hMax = sMax = vMax = 0
    phMin = psMin = pvMin = phMax = psMax = pvMax = 0

    while(1):
        # Get current positions of all trackbars
        hMin = cv.getTrackbarPos('HMin', 'image')
        sMin = cv.getTrackbarPos('SMin', 'image')
        vMin = cv.getTrackbarPos('VMin', 'image')
        hMax = cv.getTrackbarPos('HMax', 'image')
        sMax = cv.getTrackbarPos('SMax', 'image')
        vMax = cv.getTrackbarPos('VMax', 'image')

        # Set minimum and maximum HSV values to display
        lower = np.array([hMin, sMin, vMin])
        upper = np.array([hMax, sMax, vMax])

        # Convert to HSV format and color threshold
        hsv = cv.cvtColor(f, cv.COLOR_BGR2HSV)
        mask = cv.inRange(hsv, lower, upper)
        result = cv.bitwise_and(f, f, mask=mask)

        # Print if there is a change in HSV value
        if((phMin != hMin) | (psMin != sMin) | (pvMin != vMin) | (phMax != hMax) | (psMax != sMax) | (pvMax != vMax) ):
            print("(hMin = %d , sMin = %d, vMin = %d), (hMax = %d , sMax = %d, vMax = %d)" % (hMin , sMin , vMin, hMax, sMax , vMax))
            phMin = hMin
            psMin = sMin
            pvMin = vMin
            phMax = hMax
            psMax = sMax
            pvMax = vMax

        # Display result image
        cv.imshow('image', result)
        if cv.waitKey(10) & 0xFF == ord('q'):
            break

    cv.destroyAllWindows()

def resolveVexGoalByChevrons(frame, show=None):
    # frame = cv.blur(frame, (3,3))
    frame_canny = cv.Canny(frame, 100, 200)
    # cv.imshow("tit", frame_canny)

    frame_cpy = cv.cvtColor(frame_canny, cv.COLOR_GRAY2BGR)
    
    lines = cv.HoughLinesP(frame_canny, 1, np.pi / 90, 10, minLineLength=0, maxLineGap=5)
    
    print(f"{len(lines)} lines found")
    def angleType(line):
        if(abs(abs(line[5]) - .87) < .15):
            return 1 if line[5] < 0 else 2
        if(abs(abs(line[5]) - 1.042) < .15):
            return 3 if line[5] < 0 else 4
        return 0

    def lineTransform(line):
        x0 = line[0][0]
        y0 = line[0][1]
        x1 = line[0][2]
        y1 = line[0][3]
        d = ((x1-x0)**2 + (y1-y0)**2)

        if(x1 == x0):
            r = math.inf
        else:
            r = math.atan((y1 - y0) / (x1 - x0))
        return (x0, y0, x1, y1, d, r)

    lines = list(map(lineTransform, lines))

    lineSort = [[],[],[],[],[]]
    names = ["rejected", "far_left", "far_right", "near_left", "near_right"]
    colors = [(100,0,0), (0,255,0), (0,0,255), (0,255,128), (0, 128, 0)]

    for l in lines:
        lineSort[angleType(l)].append(l)

    for i in range(len(lineSort)):
        print(f"Line group {names[i]} has {len(lineSort[i])} members")
        for l in lineSort[i]:
            cv.line(frame_cpy, (l[0], l[1]), (l[2], l[3]), colors[i], thickness=1, lineType=cv.LINE_AA)

    # find line groupings

    if show is not None:
        cv.imshow("Cheveron", frame_cpy)

        if show == SHOW_BLOCK:
            while True:
                if cv.waitKey(1) & 0xFF == ord('q'):
                    break   

    # TODO - return actual goal position
    return (0,0)

def resolveVexGoalByGreen(frame, show=None):

    if show is not None:
        hsv = cv.cvtColor(frame, cv.COLOR_BGR2HSV)
        mask = cv.inRange(hsv, np.array([30, 30, 128]), np.array([128, 128, 240]))
        frame_cpy = cv.bitwise_and(frame, frame, mask=mask)

        # invert mask
        # mask = (255 - mask)

        contours, _ = cv.findContours(mask, 1, 2)

        # print(f"Countours found: {len(contours)}")
        if len(contours) != 0:
            contour_max = max(contours, key=cv.contourArea)

            x,y,w,h = cv.boundingRect(contour_max)
            cv.rectangle(frame_cpy,(x,y),(x+w,y+h),(0,255,0),2)
            cv.rectangle(mask,(x,y),(x+w,y+h),(0,255,0),2)

            M = cv.moments(contour_max)
            cx = int(M['m10']/M['m00'])
            cy = int(M['m01']/M['m00'])

            print(f"ByGreen: found goal at ({cx}, {cy})")


        cv.imshow("Mask", mask)
        cv.imshow("Green", frame_cpy)

        if show == SHOW_BLOCK:
            while True:
                if cv.waitKey(1) & 0xFF == ord('q'):
                    break   

    # TODO - return actual goal position
    return (0,0)

def resolveVexGoalByVex(frame, show=None):
    frame_cpy = cv.cvtColor(frame, cv.COLOR_GRAY2BGR)

    if show is not None:
        cv.imshow("Vex", frame_cpy)

        if show == SHOW_BLOCK:
            while True:
                if cv.waitKey(1) & 0xFF == ord('q'):
                    break   

    # TODO - return actual goal position
    return (0,0)

def cv_main():
    print("CV_MAIN")

    # f = cv.imread('PY/CV/dev/vexuGoalBad.jpg', cv.IMREAD_UNCHANGED)
    # f_in = cv.cvtColor(f, cv.COLOR_BGR2GRAY)
    # resolveVexGoalByChevrons(f_in, show=SHOW_BLOCK)
    # resolveVexGoalByGreen(f, show=SHOW_BLOCK)
    # resolveVexGoalByVex(f_in, show=SHOW_BLOCK)
    # exit(0)

    # from camera
    # capture = cv.VideoCapture(0)

    # from file
    capture = cv.VideoCapture("PY/CV/dev/out_vid.avi")

    if not capture.isOpened():
        print("Camera could not be opened")
        exit()
    print("Camera connected")

    while True:
        r, f = capture.read()

        if not r:
            break

        f_in = cv.cvtColor(f, cv.COLOR_BGR2GRAY)
        cv.imshow("gray", f_in)
        if cv.waitKey(1) & 0xFF == ord('q'):
            break  

        # resolveVexGoalByChevrons(f_in, show=SHOW_NO_BLOCK)
        resolveVexGoalByGreen(f, show=SHOW_NO_BLOCK)
        # resolveVexGoalByVex(f_in, show=SHOW_NO_BLOCK)

        time.sleep(.05)

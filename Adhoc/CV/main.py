from typing import Counter, Tuple
import cv2 as cv
import numpy as np
import math
import time
import itertools

from numpy.core.fromnumeric import resize

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
    capture = cv.VideoCapture(2, cv.CAP_DSHOW)
    out = cv.VideoWriter(f'out_vid.{round(time.time())}.avi', cv.VideoWriter_fourcc('M', 'J', 'P', 'G'), 10, (int(capture.get(3)), int(capture.get(4))))

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
    cv.namedWindow('sliders')

    def nothing(x):
        pass

    scaleFactor = .6
    dims = (int(f.shape[1] * scaleFactor), int(f.shape[0] * scaleFactor))
    f = cv.resize(f, dims)

    # Create trackbars for color change
    # Hue is from 0-179 for Opencv
    cv.createTrackbar('HMin', 'sliders', 0, 179, nothing)
    cv.createTrackbar('SMin', 'sliders', 0, 255, nothing)
    cv.createTrackbar('VMin', 'sliders', 0, 255, nothing)
    cv.createTrackbar('HMax', 'sliders', 0, 179, nothing)
    cv.createTrackbar('SMax', 'sliders', 0, 255, nothing)
    cv.createTrackbar('VMax', 'sliders', 0, 255, nothing)

    # Set default value for Max HSV trackbars
    cv.setTrackbarPos('HMax', 'sliders', 179)
    cv.setTrackbarPos('SMax', 'sliders', 255)
    cv.setTrackbarPos('VMax', 'sliders', 255)

    # Initialize HSV min/max values
    hMin = sMin = vMin = hMax = sMax = vMax = 0
    phMin = psMin = pvMin = phMax = psMax = pvMax = 0

    while(1):
        # Get current positions of all trackbars
        hMin = cv.getTrackbarPos('HMin', 'sliders')
        sMin = cv.getTrackbarPos('SMin', 'sliders')
        vMin = cv.getTrackbarPos('VMin', 'sliders')
        hMax = cv.getTrackbarPos('HMax', 'sliders')
        sMax = cv.getTrackbarPos('SMax', 'sliders')
        vMax = cv.getTrackbarPos('VMax', 'sliders')

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
        cv.imshow('sliders', f)
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
        # if len(contours) != 0:
        #     contour_max = max(contours, key=cv.contourArea)

        #     x,y,w,h = cv.boundingRect(contour_max)
        #     cv.rectangle(frame_cpy,(x,y),(x+w,y+h),(0,255,0),2)
        #     cv.rectangle(mask,(x,y),(x+w,y+h),(0,255,0),2)

        #     M = cv.moments(contour_max)
        #     cx = int(M['m10']/M['m00'])
        #     cy = int(M['m01']/M['m00'])

        #     print(f"ByGreen: found goal at ({cx}, {cy})")

        contours.sort(key=cv.contourArea)

        for i in range(int((len(contours)*.9)//1), len(contours)):
            color = (0, 0, 255) if i != (len(contours) - 1) else (0, 255, 0)
            x,y,w,h = cv.boundingRect(contours[i])
            cv.rectangle(frame_cpy,(x,y),(x+w,y+h),color,2)
            # cv.rectangle(mask,(x,y),(x+w,y+h),color,2)



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

def hsvTesting(frame, video=None):
    t = list(range(0, 257, 16))
    regions = []
    for i in range(len(t) - 2):
        regions.append((t[i], t[i+2]-1))

    # print(f"Resolved {len(regions)**2} regions")
    # print(f"Regions: {regions}")

    # todo - auto resolve
    totalHeight = 1080*.95
    totalWidth = 1920*.95

    segmentHeight = totalHeight / len(regions)
    segmentWidth = totalWidth / len(regions)

    heightScale = segmentHeight / frame.shape[0]
    widthScale = segmentWidth / frame.shape[1]
    scaleFactor = min(widthScale, heightScale)

    f = cv.cvtColor(frame, cv.COLOR_BGR2HSV)

    validContours = []
    validMasks = []
    validImages = []


    rows = []
    for region1 in regions:
        if region1[0] > 179:
            continue
        if region1[1] > 179:
            region1 = (region1[0], 179)
        chunks = []
        for region2 in regions:
            # print(f"Region {region1} {region2}")
            # continue

            # apply region and region 2 thresholds
            mask = cv.inRange(f, np.array([region1[0], region2[0], 0]), np.array([region1[1], region2[1], 255]))

            contours, heirarchy = cv.findContours(mask, cv.RETR_LIST, cv.CHAIN_APPROX_NONE)
            # cv.drawContours(frame, contours, -1, (0,255,0), 3)
            # cv.imshow('cnts', frame)

            f_cpy = cv.cvtColor(mask, cv.COLOR_GRAY2BGR)

            # print(f"Regions: {region1} {region2} --> {len(contours)}")
            for c in contours:
                actualArea = cv.contourArea(c)
                if(actualArea < 4096):
                    # too small ( < 32x32 px)
                    continue

                x,y,w,h = cv.boundingRect(c)

                height_ratio = (w/h) - .702
                if abs(height_ratio) > .5:
                    # wrong shape
                    continue

                # pixels in contour to rectangle size
                #   effectively, holey-ness and regularness check
                fullness_ratio = actualArea/(w*h)
                if fullness_ratio < .50:
                    # too sparse
                    continue


                cv.rectangle(f_cpy, (x,y), (x+w,y+h), (0,255,0), 7)
                # cv.drawContours(f_cpy, [c], 0, (0, 255, 0), 3)
                validContours.append(((region1, region2), actualArea, (x,y,w,h), (actualArea, w*h, w/h, fullness_ratio, height_ratio)))
                validMasks.append(mask)
                # print(f"Contour match: {validContours[-1]}")

            # cv.imshow('imtemp', f_cpy)
            # while True:
            #     if cv.waitKey(10) & 0xFF == ord('q'):
            #         break
            dims = (int(f_cpy.shape[1] * scaleFactor), int(f_cpy.shape[0] * scaleFactor))
            f_small = cv.resize(f_cpy, dims)

            chunks.append(f_small)

            if validContours and ((region1, region2) == validContours[-1][0]):
                validImages.append(f_cpy)

        rows.append(cv.hconcat(chunks))
        # print("Row done")
    total = cv.vconcat(rows)

    print("Contours:")
    for c in validContours:
        print(f"  {c}")
    print(f"Number of valid contours = {len(validContours)}")
    print(f"Number images with matches = {len(validImages)}")

    # magic layout function
    h = 1
    w = 1
    target_ratio = totalWidth / totalHeight
    while h*w < len(validImages):
        d_h = abs((w / (h+1)) - target_ratio)
        d_w = abs(((w + 1) / h) - target_ratio)

        if d_h > d_w:
            w += 1
        else:
            h += 1

    print(f"Optimal Layout: w:{w} h:{h} r:{w/h} target:{target_ratio}")
    s_h = (totalHeight / h) / frame.shape[1]
    s_w = (totalWidth / w) / frame.shape[0]
    scaleFactor = min(s_h, s_w)

    targetDims = (int(frame.shape[1]*scaleFactor), int(frame.shape[0]*scaleFactor))

    def shapesGen():
        for c in validImages:
            yield c
        while True:
            yield frame

    print(f"Scaling {scaleFactor} to {targetDims}")

    rows = []
    itr = iter(shapesGen())
    for _ in range(h):
        chunks = []
        for _ in range(w):
            chunks.append(cv.resize(next(itr), targetDims))
        rows.append(cv.hconcat(chunks))
    focus = cv.vconcat(rows)


    # some work with the centers of the regions
    x_centers = list(map(lambda x :(x[2][0] + x[2][2]/2), validContours))
    y_centers = list(map(lambda x :(x[2][1] + x[2][3]/2), validContours))
    areas = list(map(lambda x :(x[3][0]), validContours))
    fullness_ratios = list(map(lambda x :(x[3][3]), validContours))
    height_ratios = list(map(lambda x: (x[3][4]), validContours))

    scores = [x * abs(1-y) for x,y in zip(fullness_ratios, height_ratios)]

    def weightedAverage(iterable, weights):
        try:
            return sum(x * y for x,y in zip(iterable, weights)) / sum(weights)
        except ZeroDivisionError:
            return math.inf

    center = (
        int(weightedAverage(x_centers, scores)),
        int(weightedAverage(y_centers, scores))
    )
    frame_annotated = frame.copy()
    cv.circle(frame_annotated, center, 20, (0,0,255), 3)


    # now to actually do proper edge detection on the images
    #   first get masks of the bounding rectangles

    # magic layout function
    h = 1
    w = 1
    target_ratio = totalWidth / totalHeight
    while h*w < len(validContours):
        d_h = abs((w / (h+1)) - target_ratio)
        d_w = abs(((w + 1) / h) - target_ratio)

        if d_h > d_w:
            w += 1
        else:
            h += 1

    print(f"Optimal Layout: w:{w} h:{h} r:{w/h} target:{target_ratio}")
    s_h = (totalHeight / h) / frame.shape[1]
    s_w = (totalWidth / w) / frame.shape[0]
    scaleFactor = min(s_h, s_w)
    targetDims = (int(frame.shape[1]*scaleFactor), int(frame.shape[0]*scaleFactor))

    print(f"Scaling {scaleFactor} to {targetDims}")

    v_points = []

    def masksGen():
        assert(len(validContours) == len(validMasks))
        for contour, mask in zip(validContours, validMasks):
            x_r,y_r,w_r,h_r = contour[2]

            z = np.zeros(mask.shape, np.uint8)

            for y in range(y_r, y_r + h_r):
                for x in range(x_r, x_r + w_r):
                    z[y][x] = 255
            yield z

    def cropGen():
        for mask in masksGen():
            mask = mask.astype(np.uint8)
            yield cv.bitwise_and(frame, frame, mask=mask)

    def shapesGen():
        for img in cropGen():
            img_gray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
            img_blur = cv.GaussianBlur(img_gray, (3,3), 0)
            im_canny = cv.Canny(img_blur, 20, 50)
            # TODO - try houghLines instead of houghLinesP
            lines = cv.HoughLinesP(im_canny, 1, np.pi / 90, 5, minLineLength=50, maxLineGap=10)
            if lines is None:
                lines = []
            if len(lines) > 15:
                # too many lines
                continue
            new_img = img.copy()
            lines_better = []
            lines_mask = []
            for line in lines:
                x0, y0, x1, y1 = line[0]

                if x1 == x0:
                    # vertical line
                    r = math.inf
                else:
                    r = math.atan((y1 - y0) / (x1 - x0))

                lines_better.append((x0, y0, x1, y1, r))
                lines_mask.append(False)

            for i in range(len(lines_better)):
                rA = lines_better[i][-1]

                if abs(abs(rA) - 1.042) > .525:
                    continue

                # if abs(rA) > (1.4707):
                #     continue
                # if abs(rA) < (.1):
                #     continue

                for j in range(i+1, len(lines_better)):
                    rB = lines_better[j][-1]

                    if abs(abs(rB) - 1.042) > .525:
                        continue

                    # if abs(rB) > (1.4707):
                    #     continue
                    # if abs(rB) < (.1):
                    #     continue

                    rDiff = abs(rA - rB)
                    # print(rDiff)
                    if abs(rDiff - 1.7453) < .15:
                        # these lines are about 80 deg angled
                        # so they probably mark a V
                        lines_mask[i] = True
                        lines_mask[j] = True

            # print(f"--{len(lines_better)}--")
            # for i in range(len(lines_better)):
            #     if abs(abs(r) - 1.042) < .15:
            #         print(f"Possible Line: {lines_better[i]} marked {lines_mask[i]}")
            #     else:
            #         print(f"Ordinary Line: {lines_better[i]} marked {lines_mask[i]}")

            leftLines = []
            rightLines = []
            for line,line_m in zip(lines_better, lines_mask):
                if line_m:
                    if line[4] > 0:
                        rightLines.append(line)
                    else:
                        leftLines.append(line)

            for l_line in leftLines:
                for r_line in rightLines:
                    # calculate intersection
                    # small chance for a problem here if
                    #   one line r = -pi/2 and other f = pi/2
                    #       (parallel lines w/ opposite slopes)
                    #       note, this does not apply to a general
                    #           -x and +x pair

                    x1,y1,x2,y2,_ = l_line
                    x3,y3,x4,y4,_ = r_line

                    denom = (((x1-x2)*(y3-y4)) - ((y1-y2)*(x3-x4)))

                    if denom == 0:
                        print(f"0 denom?: {l_line} {r_line}")
                        continue

                    x_n = (((x1*y2) - (y1 * x2))*(x3-x4)) - ((x1-x2)*((x3*y4) - (y3*x4)))
                    y_n = (((x1*y2) - (y1 * x2))*(y3-y4)) - ((y1-y2)*((x3*y4) - (y3*x4)))

                    intersection = (
                        (x_n / denom),
                        (y_n / denom)
                    )

                    # print(f"Intersection: {intersection}")
                    cv.circle(frame_annotated, (int(intersection[0]), int(intersection[1])), 20, (255,128,128), 3)

                    v_points.append(intersection)

            for i in range(len(lines_better)):
                x0, y0, x1, y1, r = lines_better[i]

                if lines_mask[i] == True:
                    c = (0,0,255)
                    cv.line(frame_annotated, (x0,y0),(x1,y1), (0,0,128), 3)
                else:
                    c = (128, 0, 0)

                cv.line(new_img, (x0, y0), (x1, y1), c, thickness=3)
            # cv.imshow('check-gray', img_gray)
            # cv.imshow('check-canny', im_canny)
            # cv.imshow('check', new_img)
            # while True:
            #     if cv.waitKey(10) & 0xFF == ord('q'):
            #         break
            yield new_img
        while True:
            yield frame

    rows = []
    itr = iter(shapesGen())
    for _ in range(h):
        chunks = []
        for _ in range(w):
            chunks.append(cv.resize(next(itr), targetDims))
        rows.append(cv.hconcat(chunks))
    masks_group = cv.vconcat(rows)

    if len(v_points) > 0:
        # find the mean of the points
        v_points_x = [x[0] for x in v_points]
        v_points_y = [y[1] for y in v_points]

        x_m = int(round(sum(v_points_x) / len(v_points_x)))
        y_m = int(round(sum(v_points_y) / len(v_points_y)))

        print(f"Intersection average: {(x_m, y_m)}")

        # annotate on images
        cv.circle(frame_annotated, (x_m, y_m), 20, (0,255,0), 4)


    cv.imshow('thresholds', total)
    cv.imshow('img-focus', focus)
    cv.imshow('mask-focus', masks_group)
    cv.imshow('annotated', frame_annotated)
    if video:
        video.write(frame_annotated)
    else:
        while True:
            if cv.waitKey(10) & 0xFF == ord('q'):
                break

def hsvTesting2(frame):
    t = list(range(0, 257, 16))
    regions = []
    for i in range(len(t) - 2):
        regions.append((t[i], t[i+2]-1))

    f = cv.cvtColor(frame, cv.COLOR_BGR2HSV)

    height, width, _ = f.shape

    heatmap_table = np.zeros((height, width, 1), np.float32)


    for region1 in regions:
        if region1[0] > 60:
            continue
        if region1[1] > 179:
            region1 = (region1[0], 179)
        for region2 in regions:
            # apply region and region 2 thresholds
            mask = cv.inRange(f, np.array([region1[0], region2[0], 0]), np.array([region1[1], region2[1], 255]))

            contours, _ = cv.findContours(mask, cv.RETR_LIST, cv.CHAIN_APPROX_NONE)

            print(f"{len(contours)} contours found")

            # print(f"Regions: {region1} {region2} --> {len(contours)}")
            ctr = 0
            valid = 0
            for c in contours:
                ctr+=1
                x,y,w,h = cv.boundingRect(c)

                if abs((w/h) - .7072) > .1:
                    continue
                valid += 1
                # cv.rectangle(f_cpy, (x,y), (x+w,y+h), (0,255,0), 7)

                for x_t in range(w):
                    for y_t in range(h):
                        pt_x = x + x_t
                        pt_y = y + y_t
                        if cv.pointPolygonTest(c, (pt_x, pt_y), False) >= 0:
                            heatmap_table[pt_y][pt_x] += 1

                print(f"Coloring ({ctr})/ ({len(contours)}) v: {valid}")

        print(f"ROW done")

    # heatmap = heatmap_table*(255/hm_max)

    # something fukied down here

    hm_max = np.max(heatmap_table)
    t = heatmap_table*255
    print(f"Temp max: {np.max(t)}")
    heatmap = t * (1/hm_max)

    # print(heatmap)

    print(f"HM_Max: {hm_max}, Max: {np.max(heatmap)}, Min: {np.min(heatmap)}")

    cv.imshow('heatmap', heatmap)
    while True:
        if cv.waitKey(10) & 0xFF == ord('q'):
            break

def hsvTesting3(frame, video = None):
    frames_to_show = []
    frame_annotated = frame.copy()

    frame_gray = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)

    frame_blur = cv.GaussianBlur(frame_gray, (3,3), 0)
    frame_canny = cv.Canny(frame_blur, 20, 100)

    template = cv.imread('vexuGoal_mask5.jpg', 0)

    template_canny = cv.Canny(cv.GaussianBlur(template, (5,5), 0), 5, 50)

    template_h, template_w = template.shape

    # rectangles the template matched
    template_rect = []

    def scaleGenerator(step: int = .05):
        assert(step > 0)
        i = 1.0
        while i > 0:
            yield i
            i -= step

    # TODO - is a pyramid scale more appropriate (cv.pyrUp)
    def getScaledImage(frame_s, scale):
        assert(scale > 0)
        assert(scale <= 1)
        if scale == 1:
            return frame_s
        targetDims = (
            int(frame_s.shape[1] * scale),
            int(frame_s.shape[0] * scale)
        )
        return cv.resize(frame_s, targetDims)

    for scale in scaleGenerator():
        frame_gray_scaled = getScaledImage(frame_gray, scale)
        scaled_w, scaled_h = frame_gray_scaled.shape

        if scaled_w < template_w or scaled_h < template_h:
            # image has been down scaled too small
            break

        scaled_edge = cv.Canny(cv.GaussianBlur(frame_gray_scaled, (3,3), 0), 50, 150)
        res = cv.matchTemplate(scaled_edge, template_canny, cv.TM_CCOEFF_NORMED)

        threshold = .60
        loc = np.where(res >= threshold)
        for pt in zip(*loc[::-1]):
            # print(f"Found template match at pt {pt} scale {scale}")
            template_rect.append(((int(pt[0]/scale), int(pt[1]/scale)), (int((pt[0] + template_h)/scale), int((pt[1] + template_w)/scale))))

    # print("Rectangles:")
    # print(template_rect)

    for rect in template_rect:
        cv.rectangle(frame_annotated, rect[0], rect[1], (0,64,64), 2)

    # f_green = cv.inRange(cv.cvtColor(frame, cv.COLOR_BGR2HSV), np.array([20, 50, 50]), np.array([100, 255, 255]))
    # f_green = cv.cvtColor(f_green, cv.COLOR_GRAY2BGR)
    # for rect in template_rect:
    #     cv.rectangle(f_green, rect[0], rect[1], (0,0,255), 3)


    if template_rect:
        # now try to determine the partions on the rectangles:
        # rect_centers = []
        # for up_left, low_right in template_rect:
        #     rect_centers.append(
        #         (((low_right[0] - up_left[0]) / 2),
        #         ((low_right[1] - up_left[1]) / 2))
        #     )

        template_rect.sort(key = lambda x : ((x[0][0] + x[1][0]) / 2))
        groups = []
        maxX = -1

        for up_left, low_right in template_rect:
            x = (up_left[0] + low_right[0]) / 2
            y = (up_left[1] + low_right[1]) / 2
            if x > maxX:
                groups.append([])
            groups[-1].append((x, y))
            maxX = max(maxX, low_right[0])

        print(f"Resolved {len(groups)} groups")

        for group in groups:
            sum_x = 0
            sum_y = 0

            for x, y in group:
                sum_x += x
                sum_y += y

            x_c = int(sum_x / len(group))
            y_c = int(sum_y / len(group))

            cv.circle(frame_annotated, (x_c, y_c), 15, (0, 0, 255), -1)

    # extract clustering of points

    frames_to_show.append(("original", frame))
    frames_to_show.append(("annotated", frame_annotated))
    # frames_to_show.append(("gray", frame))
    # frames_to_show.append(("template_canny", template_canny))
    # frames_to_show.append(("canny", frame_canny))
    # frames_to_show.append(("green", f_green))

    if video:
        video.write(frame_annotated)
    else:
        for title, frame in frames_to_show:
            cv.namedWindow(title, cv.WINDOW_NORMAL)
            cv.imshow(title, frame)

        while True:
            if cv.waitKey(10) & 0xFF == ord('q'):
                break
        print("-------------------------------------------")

def cv_main():
    print("CV_MAIN")
    recordVideo()

    # f = cv.imread('dev/vexuGoal.jpg', cv.IMREAD_UNCHANGED)
    # imageHSVSliders(f)
    # f_in = cv.cvtColor(f, cv.COLOR_BGR2GRAY)
    # hsvTesting(f)
    # hsvTesting3(f)
    # exit(0)
    # resolveVexGoalByGreen(f, show=SHOW_BLOCK)
    # # resolveVexGoalByChevrons(f_in, show=SHOW_BLOCK)
    # # resolveVexGoalByVex(f_in, show=SHOW_BLOCK)
    # exit(0)

    # from camera
    # capture = cv.VideoCapture(1, cv.CAP_DSHOW)

    # from file
    # capture = cv.VideoCapture("dev/out_vid.avi")
    # capture = cv.VideoCapture("dev/swirlTestVid12.mp4")
    capture = cv.VideoCapture("dev/15.1.avi")

    # r,f = capture.read()
    # cv.imwrite("frame.jpg", f)
    # exit(0)

    if not capture.isOpened():
        print("Camera could not be opened")
        exit()
    print("Camera connected")

    # out = cv.VideoWriter(f'out_vid.{round(time.time())}.avi', cv.VideoWriter_fourcc('M', 'J', 'P', 'G'), 10, (int(capture.get(3)), int(capture.get(4))))
    out = None

    while True:
        r, f = capture.read()

        if not r:
            break

        # imageHSVSliders(f)
        # f_in = cv.cvtColor(f, cv.COLOR_BGR2GRAY)
        # cv.imshow("gray", f_in)

        # resolveVexGoalByChevrons(f_in, show=SHOW_NO_BLOCK)
        # resolveVexGoalByGreen(f, show=SHOW_NO_BLOCK)
        # resolveVexGoalByVex(f_in, show=SHOW_NO_BLOCK)
        hsvTesting3(f)
        if cv.waitKey(1) & 0xFF == ord('q'):
            break

        # time.sleep(.05)

if __name__ == "__main__":
    cv_main()
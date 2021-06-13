if __name__ != "__main__":
    from PY.vexController.vexAction import VEX_jitterStart, VEX_stop
import itertools
import math
import os
import re
import select
import subprocess
from typing import Dict, List, Optional, Tuple, Union


def listMice() -> List[str]:
    f = os.listdir("/dev/input")
    f = ["/dev/input/" + m for m in f if "mouse" in m]
    return f

# determine the valid mice from the partial mice
#   uses all mice if partial not proveded
def determineValidMice(partial_mice : Optional[List[str]] = None) -> List[str]:
    if partial_mice is None:
        partial_mice = listMice()

    VEX_jitterStart()

    probablyMice = []

    for mouse in partial_mice:

        with open(mouse, 'rb') as m_file:
            r, _, _ = select.select([m_file], [], [], .2)
            if m_file in r:
                probablyMice.append(mouse)
            else:
                pass

    VEX_stop()

    return probablyMice

# given a list of USB address,
#   return the /dev/input/mouse# file associated with it:
#   input should be string usb address <bus>.<addr>[.<sub_addr>]
#       i.e. 1.2 or 1.2.1.2
def determineMiceByPath(usb_paths : List[str]) -> List[Optional[str]]:
    cwd = os.getcwd()
    try:
        os.chdir("/dev/input/by-path")
        sub = subprocess.run(['ls', '-l'], stdout=subprocess.PIPE)
    finally:
        os.chdir(cwd)
    
    # regex for extracting mouse devices
    #   first capture will be the usb address a la "1.1"
    #   second capture will be file path a la "/mouse0"
    mouseEventsRegex = "[a-zA-Z0-9 :-]+\.usb-usb-0:([0-9\.]+):[0-9\-\.a-z \>]+(\/mouse[0-9]+)"

    lines = list(sub.stdout.decode().split('\n'))

    mapping = {}

    for line in lines:
        m = re.match(mouseEventsRegex, line)
        if m is not None:
            mapping[m[1]] = m[2]

    matches = []
    for usb_addr in usb_paths:
        if usb_addr in mapping:
            matches.append("/dev/input" + mapping[usb_addr])
        else:
            matches.append(None)
    return matches

# determine the d_value given deltas and known rotation amount
#   this is implemented very lazily so don't call it alot
#   uses gradient descent to zero in on the proper d value
#   its O(bad)
def determineDFromDeltasRotation(deltas : List[float], known_rotation_rad : float) -> float:
    precision = 0.00001
    current_in = 5000
    step_size = current_in / 10

    round_ctr = 0
    delta = precision*2

    attempted = set()

    while True:
        if round_ctr % 10 == 0:
            print(f"Target: {known_rotation_rad}")

        current_in = float(input("Enter guess:"))

        raw = determineTranslationFromDelta(deltas, current_in)[2] 
        delta = raw - known_rotation_rad

        print(f"Round {round_ctr}: current_in {current_in}, resolved {raw}, delta {delta}, step {step_size}")

        if abs(delta) < precision:
            break

        round_ctr += 1
        if known_rotation_rad > 0:
            if delta < 0:
                current_in -= step_size
                if current_in <= 0:
                    current_in = precision
            else:
                current_in += step_size
        else:
            if raw > known_rotation_rad:
                current_in -= step_size
                if current_in <= 0:
                    current_in = precision
            else:
                current_in += step_size


        if (current_in, step_size) in attempted:
            step_size = step_size / 2
        attempted.add((current_in, step_size))

        if round_ctr > 200:
            print("Error incoming:")
            print(f"Deltas: {deltas}")
            print(f"Know_rotation: {known_rotation_rad}")
            raise ValueError(f"Round counter too large: {round_ctr}")

        if step_size < (precision / 100):
            print("Error incoming:")
            print(f"Deltas: {deltas}")
            print(f"Known_rotation: {known_rotation_rad}")
            raise ValueError(f"Step too small: {step_size}")

    print(f"current_in: {current_in}")
    return current_in

# given the deltas and the known d_value
#   return a tuple
#       x_change (relative to original orientation)
#       y_change (relative to original orientation)
#       rotation change rad (relative to original orientation)
def determineTranslationFromDelta(deltas : List[float], d_value : float, is_test : bool = False) -> Union[Dict[str, float], Tuple[float, float, float]]:
    if sum(deltas) == 0:
        if is_test:
            return {}
        # no movement case
        return (0,0,0)

    # unpack the deltas into something easier to work with
    m1x, m1y, m2x, m2y = deltas

    # find the magnititude of rotation
    a1_r = (m1x**2 + m1y**2)**.5
    a2_r = (m2x**2 + m2y**2)**.5

    # apply cramer's rule for line intersection
    # don't need to do explicit transform cause
    #   we have the perpendicular vector to begin with
    #   TODO - fix later (remove r#_#, a#, b#)
    # find the r vectors
    r1_x = m1y
    r1_y = -m1x
    r2_x = m2y
    r2_y = -m2x
    a1 = -r1_y
    b1 = r1_x
    a2 = -r2_y
    b2 = r2_x
    c2 = -r2_y * d_value

    # find the anti-rotation point
    # denom = m1x * m2y - m1y * m2x
    denom = (a1 * b2) - (b1 * a2)

    # print(f"R: ({r1_x}, {r1_y}) ({r2_x}, {r2_y})")
    # print(f"a: ({a1}, {b1}) ({a2}, {b2})")
    # print(f"Denom: {denom}")
    # print(f"C2: {c2}")

    if abs(denom) < .0001:
        # degenerate cases
        # the requisite lines are parallel
        # either:
        #   the rotation point is at infinity
        #   effectively, no rotation occurred in the period
        # -or-
        #   the rotation point is in line with both mice

        if (abs(m1x - m2x) < .0001) and (abs(m1y - m2y) < .0001):
            # the magnitude and direction is effectively the same,
            #   so no rotation occurred, just translation
            if is_test:
                return {
                    'd' : d_value,
                    'm1x' : m1x,
                    'm1y' : m1y,
                    'm2x' : m2x,
                    'm2y' : m2y,
                    'm1pos_end_x' : m1x,
                    'm1pos_end_y' : m1y,
                }
            return (m1x, m1y, 0)
        
        # now we know a rotation occurred and it is in line with the mice
        #   check if the rotation is directly on the mice
        if abs(a1_r) < .0001:
            # rotating directly on m1
            rot_x = 0
            rot_y = 0
        elif abs(a2_r) < .0001:
            # rotating directly on m2
            rot_x = d_value
            rot_y = 0
        elif abs(a1_r - a2_r) < .0001:
            # rotating directly between the mice
            rot_x = d_value / 2
            rot_y = 0 
        else:
            # rotation about some other point
            #  but still in line with the mice
            rot_x = -(d_value / ((a2_r / a1_r) - 1))
            rot_y = 0
    else:
        # rot_x = (-m1y * c2) / denom
        # rot_y = (m1x * c2) / denom
        rot_x = -(b1 * c2) / denom
        rot_y = (a1 * c2) / denom

    # print(f"rotation pos: {rot_x} {rot_y}")

    # draw vectors from r point to m points
    r1x = 0 - rot_x
    r1y = 0 - rot_y
    r2x = d_value - rot_x
    r2y = 0 - rot_y

    # find the lengths of the legs
    l1 = (r1x**2 + r1y**2)**.5
    l2 = (r2x**2 + r2y**2)**.5
    # print(f"l-vals: {l1} {l2}")

    # find the inner angle on the triangle
    #   not used for anything really
    if is_test:
        try:
            theta = math.atan2(
                (-rot_y * (d_value - rot_x)) - (-rot_x * -rot_y),
                (-rot_x * (d_value - rot_x)) + (-rot_y * -rot_y)
            )
            theta2 = math.acos(
                (r1x * r2x + r1y * r2y) /
                (l1 * l2)
            )
            # print(f"theta: {theta2} {theta}")
            assert(abs(abs(theta2) - abs(theta)) < .0001)
        except ZeroDivisionError:
            pass

    # find the full rotation circumference
    c1 = 2 * math.pi * l1
    c2 = 2 * math.pi * l2
    # print(f"c-vals: {c1} {c2}")

    # find rotation amount
    #   technically can optimise whole thing to rotationRadians = a1 / l1
    try:
        rotation_amt_1 = a1_r / c1
        rotation_amt_2 = a2_r / c2
        # print(f"rot_amt: {rotation_amt_1} {rotation_amt_2}")
        # print(f"a_vals: {a1_r} {a2_r}")

        # assert(abs(rotation_amt_1 - rotation_amt_2) < .0001)
        rotationRadians = (rotation_amt_1 + rotation_amt_2) * math.pi
    except ZeroDivisionError:
        # can't both be zero, that would be caught earlier
        if c1 == 0:
            rotationRadians = a2_r / c2 * 2 * math.pi
        else:
            rotationRadians = a1_r / c1 * 2 * math.pi

    # need to determine the rotation direction
    #   this is given by rotating vector r1 into a1
    #       and getting a direction
    #   or much simpler
    #       since a1 is perp to r1
    #       find the positive rotation perp of r1
    #       and see if it matches a1
    #       if not, negative rotation

    isPositiveRotation = True

    if r1x != 0 or r1y != 0:
        a_ref_x = -r1y
        a_rey_y = r1x

        if (abs(a_ref_x) > .0001) and  (abs(m1x) > .0001) and ((a_ref_x > 0) != (m1x > 0)):
            isPositiveRotation = False
        if (abs(a_rey_y) > .0001) and  (abs(m1y) > .0001) and ((a_rey_y > 0) != (m1y > 0)):
            isPositiveRotation = False

        # print(f"m_vals {m1x} {m1y}")
        # print(f"r1 {r1x} {r1y}")
    else:
        a_ref_x = -r2y
        a_rey_y = r2x

        if (abs(a_ref_x) > .0001) and  (abs(m2x) > .0001) and ((a_ref_x > 0) != (m2x > 0)):
            isPositiveRotation = False
        if (abs(a_rey_y) > .0001) and  (abs(m2y) > .0001) and ((a_rey_y > 0) != (m2y > 0)):
            isPositiveRotation = False

        # print(f"m_vals {m2x} {m2y}")
        # print(f"r2 {r2x} {r2y}")

    # print(f"a ref {a_ref_x} {a_rey_y}")
    # print(f"ispositive rotrad {isPositiveRotation} {rotationRadians}")
    if (isPositiveRotation) != (rotationRadians > 0):
        # sign mismatch
        rotationRadians = -rotationRadians

    # finally, get the ending mouse 0 point
    newX = (math.cos(rotationRadians) * -rot_x) - (math.sin(rotationRadians) * -rot_y) + rot_x
    newY = (math.sin(rotationRadians) * -rot_x) + (math.cos(rotationRadians) * -rot_y) + rot_y


    # return [d_value, m1x, m1y, m2x, m2y, a1, a2]
    if is_test:
        return {
            "d" : d_value,
            "m1x" : m1x,
            "m1y" : m1y,
            "m2x" : m2x,
            "m2y" : m2y,
            "rotation_pos_x" : rot_x,
            "rotation_pos_y" : rot_y,
            "rotation_amt_rad" : rotationRadians,
            "m1pos_end_x" : newX,
            "m1pos_end_y" : newY,
            "l1" : l1,
            "l2" : l2,
            "c1" : c1,
            "c2" : c2,
            "a1" : a1_r,
            "a2" : a2_r,
        }
    return (newX, newY, rotationRadians)

# get the translation values given the ending values
#   for debugging purposes
def doTranslationBackwards(testCase, rotationAmount, m1pos, m2pos) -> List[float]:

    m1pos_x, m1pos_y = m1pos
    m2pos_x, m2pos_y = m2pos

    assert(m1pos_y == 0)
    assert(m2pos_y == 0)
    assert(m1pos_x < m2pos_x)

    rotationPos = testCase

    ########
    # find d
    d_value = ((m1pos_x - m2pos_x)**2 + (m1pos_y - m2pos_y)**2)**.5

    ########
    # conversion
    rotationRadians = math.radians(rotationAmount)

    # safety
    if rotationRadians == 0:
        return {
            "d" : d_value,
            "m1x" : 0,
            "m1y" : 0,
            "m2x" : 0,
            "m2y" : 0,
            "rotation_amt_rad" : 0,
            "m1pos_end_x" : 0,
            "m1pos_end_y" : 0,
            "m2pos_end_x" : d_value,
            "m2pos_end_y" : 0,
            "a1" : 0,
            "a2" : 0,
            "c1" : 0,
            "c2" : 0,
            "l1" : 0,
            "l2" : 0,
        }

    ########
    # find new mouse positions
    s = math.sin(rotationRadians)
    c = math.cos(rotationRadians)

    # get the mouse points with rotation as origin
    m1_rot = (m1pos_x - rotationPos[0], m1pos_y - rotationPos[1])
    m2_rot = (m2pos_x - rotationPos[0], m2pos_y - rotationPos[1])

    m1_new = (m1_rot[0] * c - m1_rot[1] * s, m1_rot[0] * s + m1_rot[1] * c)
    m2_new = (m2_rot[0] * c - m2_rot[1] * s, m2_rot[0] * s + m2_rot[1] * c)

    m1pos_end = (m1_new[0] + rotationPos[0], m1_new[1] + rotationPos[1])
    m2pos_end = (m2_new[0] + rotationPos[0], m2_new[1] + rotationPos[1])

    ########
    # finding lengths
    l1 = (m1_rot[0]**2 + m1_rot[1]**2)**.5
    l2 = (m2_rot[0]**2 + m2_rot[1]**2)**.5

    assert(l1 >= 0)
    assert(l2 >= 0)

    ########
    # finding circumferences
    c1 = 2 * math.pi * l1
    c2 = 2 * math.pi * l2

    ########
    # finding angular amounts
    a1 = c1 * (rotationRadians / math.radians(360))
    a2 = c2 * (rotationRadians / math.radians(360))

    ########
    # find the offset thetas from m#x+ to R
    if m1_rot[0] == 0:
        if m1_rot[1] == 0:
            t1 = 0
        else:
            t1 = math.radians(90) * (m1_rot[1]/abs(m1_rot[1]))
    else:
        t1 = math.atan2(m1_rot[1], m1_rot[0])
    
    if m2_rot[0] == 0:
        if m2_rot[1] == 0:
            t2 = 0
        else:
            t2 = math.radians(90) * (m2_rot[1]/abs(m2_rot[1]))
    else:
        t2 = math.atan2(m2_rot[1], m2_rot[0])
    
    ########
    # find the appropriate gamma angle
    if rotationAmount >= 0:
        g1 = t1 + math.radians(90)
        g2 = t2 + math.radians(90)
    else:
        g1 = t1 - math.radians(90)
        g2 = t2 - math.radians(90)

    # print(f"  m1_pos_end: {m1pos_end}")
    # print(f"  m2_pos_end: {m2pos_end}")
    # print(f"  m1_rot: {m1_rot}")
    # print(f"  m2_rot: {m2_rot}")
    # print(f"  l1: {l1}")
    # print(f"  l2: {l2}")
    # print(f"  c1: {c1}")
    # print(f"  c2: {c2}")
    # print(f"  a1: {a1}")
    # print(f"  a2: {a2}")
    # print(f"  t1: {t1}")
    # print(f"  t2: {t2}")
    # print(f"  g1: {g1}")
    # print(f"  g2: {g2}")

    ########
    # finding the displacements to pass in
    m1x = math.cos(g1) * a1
    m1y = math.sin(g1) * a1
    m2x = math.cos(g2) * a2
    m2y = math.sin(g2) * a2

    ########
    # adjusting for rotation direction
    if rotationRadians < 0:
        m1x = -m1x
        m1y = -m1y
        m2x = -m2x
        m2y = -m2y
        a1 = -a1
        a2 = -a2

    ########
    ########
    ########
    # return [
    #     d_value, m1pos_x, m1pos_y, m2pos_x, m2pos_y, rotationPos, rotationRadians, m1pos_end, m2pos_end,
    #     l1, l2, c1, c2, a1, a2, t1, t2, 
    # ]
    return {
        "d" : d_value,
        "m1x" : m1x, 
        "m1y" : m1y,
        "m2x" : m2x,
        "m2y" : m2y,
        "rotation_pos_x" : rotationPos[0],
        "rotation_pos_y" : rotationPos[1],
        "rotation_amt_rad" : rotationRadians,
        "m1pos_end_x" : m1pos_end[0],
        "m1pos_end_y" : m1pos_end[1],
        "m2pos_end_x" : m2pos_end[0],
        "m2pos_end_y" : m2pos_end[1],
        "l1" : l1,
        "l2" : l2,
        "c1" : c1,
        "c2" : c2,
        "a1" : a1,
        "a2" : a2,
        "t1" : t1,
        "t2" : t2,
    }

def runOdomResolverTest():
    # each case will be tried in all possible rotations
    #   and for + and - theta rotations
    # testCases lists rotation points

    m1StartPos = (0,0)
    m2StartPos = (2,0)
    # testCases = [(-1, 0), (-1, 2), (0, 2), (1, 2), (2, 2), (3, 2), (3, 0), (3, -1), (2,-1), (1, -1), (0, -1), (0, 0)]
    testCases = list(itertools.product(range(-1, 4), range(-1,1)))
    rotationAmounts = [0, 30, 45, 60, 90, 120, 135, 150]

    errorCounter = 0
    d_errorCounter = 0
    skipCounter = 0
    totalCases = 0

    for testCase in testCases:
        for rotationAmount in itertools.chain(rotationAmounts, map(lambda x : -x, rotationAmounts), [180]):
            totalCases += 1

            # if testCase == m1StartPos or testCase == m2StartPos:
            #     # for now, skipping cases of rotation directly about a mouse
            #     skipCounter += 1
            #     continue

            # if testCase[1] == 0:
            #     # skipping cases where the rotation is through the mice
            #     skipCounter += 1
            #     continue

            print(f"Starting: {testCase} {rotationAmount}")

            k = doTranslationBackwards(testCase, rotationAmount, m1StartPos, m2StartPos)

            offsets = [k["m1x"], k["m1y"], k["m2x"], k["m2y"]]
            # print(f"Resolved the offsets: {offsets}")

            kk = determineTranslationFromDelta(offsets, k["d"], True)

            hasError = False

            for key,value in kk.items():
                if abs(value - k[key]) > 0.0001:
                    hasError = True
                    break

            if hasError:
                print("Error found:")
                print(f"Test case: {testCase} {rotationAmount}")
                errorCounter += 1

                for key in kk:
                    v = k[key]
                    vv = kk[key]
                    t = abs(v - vv) < .0001
                    print(f"  {' ' if t else '*'} {key}: {v} {vv}")

                if(testCase == (1, -1)) and (rotationAmount == 90):
                    print("Flagged case, exiting early")
                    exit(0)

                if(testCase == (0, -1)) and (rotationAmount == 90):
                    print("Flagged case, exiting early")
                    exit(0)

            if rotationAmount == 0:
                # can't use 0 rotation to recover d
                continue
            if rotationAmount < 0:
                # TODO - figure out why this is broken
                continue
            if rotationAmount in [45, 90, 135, 180]:
                # why do these break?
                #   something about the symmetry?
                continue
            d_calc = determineDFromDeltasRotation(offsets, math.radians(rotationAmount))
            if abs(d_calc - k['d']) > .0001:
                print(f"  * D error: {k['d']} {d_calc}")
                d_errorCounter += 1

    print("All test cases finished")
    print(f"{totalCases} total cases")
    print(f"{errorCounter} errors")
    print(f"{d_errorCounter} d_errors")
    print(f"{skipCounter} skipped")

    # TODO - add tests for straight translations

if __name__ == "__main__":
    runOdomResolverTest()
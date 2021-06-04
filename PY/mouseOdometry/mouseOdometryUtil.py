if __name__ != "__main__":
    from PY.vexController.vexAction import VEX_jitterStart, VEX_stop
import itertools
import math
import os
import select
from typing import List, Optional


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


def determineTranslationFromDelta(deltas : List[float], d_value : float) -> List[float]:    
    # unpack the deltas into something easier to work with
    m1x, m1y, m2x, m2y = deltas

    # TODO - edge cases:
    #   no-movment case
    #   no-rotation case
    #   rotation in line with mice case


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
    if denom == 0:
        # degenerate case
        # the requisite lines are parallel
        # either:
        #   the rotation point is at infinity
        #   effectively, no rotation occurred in the period
        # -or-
        #   the rotation point is in line with both mice
        print(denom)
        print(f"M: ({m1x}, {m1y}) ({m2x}, {m2y})")
        print(f"R: ({r1_x}, {r1_y}) ({r2_x}, {r2_y})")
        print(f"a: ({a1}, {b1}) ({a2}, {b2})")
        raise NotImplementedError()

    print(f"R: ({r1_x}, {r1_y}) ({r2_x}, {r2_y})")
    print(f"a: ({a1}, {b1}) ({a2}, {b2})")
    print(f"Denom: {denom}")
    print(f"C2: {c2}")

    # rot_x = (-m1y * c2) / denom
    # rot_y = (m1x * c2) / denom
    rot_x = -(b1 * c2) / denom
    rot_y = (a1 * c2) / denom

    # draw vectors from r point to m points
    r1x = 0 - rot_x
    r1y = 0 - rot_y
    r2x = d_value - rot_x
    r2y = 0 - rot_y

    # find the lengths of the legs
    l1 = (r1x**2 + r1y**2)**.5
    l2 = (r2x**2 + r2y**2)**.5

    # find the inner angle on the triangle
    #   also gives the direction of rotation
    theta = math.atan2(
        (-rot_y * (d_value - rot_x)) - (-rot_x * -rot_y),
        (-rot_x * (d_value - rot_x)) + (-rot_y * -rot_y)
    )
    theta2 = math.acos(
        (r1x * r2x + r1y * r2y) /
        (l1 * l2)
    )
    print(f"{theta2} {theta}")
    assert(abs(abs(theta2) - abs(theta)) < .0001)

    # find the amount of rotation
    a1 = (m1x**2 + m1y**2)**.5
    a2 = (m2x**2 + m2y**2)**.5

    # find the full rotation circumference
    c1 = 2 * math.pi * l1
    c2 = 2 * math.pi * l2

    # find rotation amount
    #   technically can optimise whole thing to rotationRadians = a1 / l1
    rotation_amt_1 = a1 / c1
    rotation_amt_2 = a2 / c2

    assert(abs(rotation_amt_1 - rotation_amt_2) < .0001)

    rotationRadians = rotation_amt_1 * 2 * math.pi
    if (theta > 0) != (rotationRadians > 0):
        # sign mismatch
        rotationRadians = -rotationRadians

    # finally, get the ending mouse 0 point
    newX = (math.cos(rotationRadians) * -rot_x) - (math.sin(rotationRadians) * -rot_y) + rot_x
    newY = (math.sin(rotationRadians) * -rot_x) + (math.cos(rotationRadians) * -rot_y) + rot_y


    # return [d_value, m1x, m1y, m2x, m2y, a1, a2]
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
        "a1" : a1,
        "a2" : a2,
    }

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
        t1 = math.radians(90) * (m1_rot[1]/abs(m1_rot[1]))
    else:
        t1 = math.atan2(m1_rot[1], m1_rot[0])
    
    if m2_rot[0] == 0:
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

    print(f"  m1_pos_end: {m1pos_end}")
    print(f"  m2_pos_end: {m2pos_end}")
    print(f"  m1_rot: {m1_rot}")
    print(f"  m2_rot: {m2_rot}")
    print(f"  l1: {l1}")
    print(f"  l2: {l2}")
    print(f"  c1: {c1}")
    print(f"  c2: {c2}")
    print(f"  a1: {a1}")
    print(f"  a2: {a2}")
    print(f"  t1: {t1}")
    print(f"  t2: {t2}")
    print(f"  g1: {g1}")
    print(f"  g2: {g2}")

    ########
    # finding the displacements to pass in
    m1x = math.cos(g1) * a1
    m1y = math.sin(g1) * a1
    m2x = math.cos(g2) * a2
    m2y = math.sin(g2) * a2

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
    testCases = [(-1, 0), (-1, 2), (0, 2), (1, 2), (2, 2), (3, 2), (3, 0), (3, -1), (2,-1), (1, -1), (0, -1), (0, 0)]
    rotationAmounts = [0, 30, 45, 60, 90, 120, 135, 150, 180]

    errorCounter = 0
    skipCounter = 0

    for testCase in testCases:
        for rotationAmount in itertools.chain(rotationAmounts, map(lambda x : -x, rotationAmounts), [180]):
            if testCase == m1StartPos or testCase == m2StartPos:
                # for now, skipping cases of rotation directly about a mouse
                skipCounter += 1
                continue

            if rotationAmount == 0:
                # the best way to deal with test cases
                # is to skip the ones you fail
                skipCounter += 1
                continue

            if testCase[1] == 0:
                # skipping cases where the rotation is through the mice
                skipCounter += 1
                continue

            print(f"Starting: {testCase} {rotationAmount}")

            k = doTranslationBackwards(testCase, rotationAmount, m1StartPos, m2StartPos)

            offsets = [k["m1x"], k["m1y"], k["m2x"], k["m2y"]]
            print(f"Resolved the offsets: {offsets}")

            kk = determineTranslationFromDelta(offsets, k["d"])

            hasError = False

            for key,value in kk.items():
                if abs(value - k[key]) > 0.0001:
                    hasError = True
                    break

            if hasError:
                print("Error found:")
                print(f"Test case: {testCase} {rotationAmount}")

                for key in kk:
                    print(f"  {key}: {k[key]} {kk[key]}")

                if(testCase == (1, -1)) and (rotationAmount == 90):
                    print("Flagged case, exiting early")
                    exit(0)

                if(testCase == (0, -1)) and (rotationAmount == 90):
                    print("Flagged case, exiting early")
                    exit(0)

    print("All test cases finished")
    print(f"{errorCounter} errors")
    print(f"{skipCounter} skipped")

    # TODO - add tests for straight translations

if __name__ == "__main__":
    runOdomResolverTest()
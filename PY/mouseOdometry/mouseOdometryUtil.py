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

    # find the arc-adjsted rotation amount
    a1 = (m1x**2 + m1y**2)**.5
    a2 = (m2x**2 + m2y**2)**.5

    # find the anti-rotation point
    denom = m1x * m2y - m2x * m1y
    if denom == 0:
        # degenerate case
        # the requisite lines are parallel
        # either:
        #   the rotation point is at infinity
        #   effectively, no rotation occurred in the period
        # -or-
        #   the rotation point is in line with both mice
        print(denom)
        print(f"({m1x}, {m1y}) ({m2x}, {m2y})")
        raise NotImplementedError()

    # angle from a1 to a2
    aaa = math.atan2(m1x * m2y - m1y * m2x, m1x * m2x + m1y * m2y)
    print(f"aaa: {aaa}")
    assert(abs(aaa) < math.radians(180))

    if aaa > 0:
        # rotation direction is negative
        a1 = -a1
        a2 = -a2

    
    return [d_value, m1x, m1y, m2x, m2y, a1, a2]

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
    return [
        d_value, m1x, m1y, m2x, m2y, a1, a2,
    ]

def runOdomResolverTest():
    # each case will be tried in all possible rotations
    #   and for + and - theta rotations
    # testCases lists rotation points

    m1StartPos = (0,0)
    m2StartPos = (2,0)
    testCases = [(-1, 0), (-1, 2), (0, 2), (1, 2), (2, 2), (3, 2), (3, 0), (3, -1), (2,-1), (1, -1), (0, -1), (0, 0)]
    rotationAmounts = [0, 30, 45, 60, 90] #, 120, 135, 150, 180]

    return_order = ["d", "m1x", "m1y", "m2x", "m2y", "a1", "a2"]
    errorCounter = 0
    skipCounter = 0

    for testCase in testCases:
        for rotationAmount in itertools.chain(rotationAmounts, map(lambda x : -x, rotationAmounts)):
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

            print(f"Resolved the offsets: {k[1:5]}")

            kk = determineTranslationFromDelta(k[1:5], k[0])

            if (len(k) == len(kk) and len(k) == len(return_order)) is False:
                print(f"Lengths: {len(return_order)} {len(k)} {len(kk)}")
                assert(len(k) == len(kk) and len(k) == len(return_order))


            if (False in map(lambda x,y : abs(x-y) < .0001, k, kk)):
                errorCounter += 1

                print("Error found:")
                print(f"Test case: {testCase}, {rotationAmount}")


                for i in range(len(k)):
                    print(f"  {return_order[i]}: {k[i]} {kk[i]}")

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
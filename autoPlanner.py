from functools import partial
import math
import itertools

isSkills = True

# TODO - proper enum
EMPTY = 0
RED = 1
BLUE = 2

# time constants


if isSkills:
    # all positions are relative to the lower left red corner
    goalLocations = {
        "A" : (4,4),
        "B" : (72,4),
        "C" : (140, 4),
        "D" : (4, 72),
        "E" : (72, 72),
        "F" : (140, 72),
        "G" : (4, 140),
        "H" : (72, 140),
        "I" : (140, 140),
    }

    # tracks unscored balls
    ballLocations = {
        "1" : (36, 24),
        "2" : (108, 24),
        "3" : (4, 36),
        "4" : (140, 36),
        "5" : (72, 48),
        "6" : (24, 72),
        "7" : (48, 72),
        "8" : (96, 72),
        "9" : (120, 72),
        "10" : (72, 96),
        "11" : (108, 4),
        "12" : (136, 108),
        "13" : (36, 120),
        "14" : (108, 108),
    }

    goalStates = {
        "A" : (BLUE, BLUE, EMPTY),
        "B" : (BLUE, EMPTY, EMPTY),
        "C" : (BLUE, BLUE, EMPTY),
        "D" : (BLUE, EMPTY, EMPTY),
        "E" : (BLUE, BLUE, BLUE),
        "F" : (BLUE, EMPTY, EMPTY),
        "G" : (BLUE, BLUE, EMPTY),
        "H" : (BLUE, EMPTY, EMPTY),
        "I" : (BLUE, BLUE, EMPTY),
    }
else:
    raise NotImplementedError()

from plannerPath import scriptStack


def stateAppend(startState, toAppend):
    if(startState[0] == EMPTY):
        return (toAppend, EMPTY, EMPTY)
    elif(startState[1] == EMPTY):
        return (startState[0], toAppend, EMPTY)
    elif(startState[2] == EMPTY):
        return (startState[0], startState[1], EMPTY)
    else:
        raise ValueError(f"Start state {startState} too full")

def validateScript(script):
    g_states = goalStates.copy()
    b_location = ballLocations.copy()
    myState = (RED, EMPTY, EMPTY)

    hasParked = False

    for k in script:
        assert(hasParked == False)
        tokens = k.split(" ")

        print(tokens)

        if tokens[0] == "cycle":
            assert(len(tokens) == 2)
            assert(tokens[1] in g_states)
            assert(BLUE not in myState)
            s = myState
            myState = g_states[tokens[1]]
            g_states[tokens[1]] = s
        elif tokens[0] == "eject":
            assert(len(tokens) == 2)
            assert(tokens[1] in ["SAFE", "FORCE"])
            myState = (EMPTY, EMPTY, EMPTY)
        elif tokens[0] == "grab":
            assert(len(tokens) == 2)
            assert(tokens[1] in b_location)
            assert(EMPTY in myState)
            b_location.pop(tokens[1])
            myState = stateAppend(myState, RED)
        elif tokens[0] == "park":
            assert(len(tokens) == 1)
            hasParked = True
        else:
            raise ValueError(f"Illegal command `{tokens[0]}`")

    assert(hasParked)

    # assert all balls placed
    assert(not b_location)
    assert(RED not in myState)

    # assert goals only red
    assert(BLUE not in itertools.chain.from_iterable(g_states.values()))

    print("Script validated successfully")

def turnMoveTo(currentPos, currentOrient, targetPos):
        vectorToTarget = (
            targetPos[0] - currentPos[0],
            targetPos[1] - currentPos[1],
        )
        try:
            targetRotation = math.atan2(vectorToTarget[1], vectorToTarget[0])
        except ZeroDivisionError:
            targetRotation = (math.pi/2) if vectorToTarget[1] > 0 else (-math.pi/2)

        rotationAmount_left = targetRotation - (currentOrient if currentOrient < targetRotation else (currentOrient - 2*math.pi))
        rotationAmount_right = -targetRotation + (currentOrient if currentOrient > targetRotation else (currentOrient + 2*math.pi))

        if rotationAmount_left < rotationAmount_right:
            rot_amt = rotationAmount_left
        else:
            rot_amt = -rotationAmount_right

        # now amount to drive
        distToTarget = ((targetPos[0] - currentPos[0])**2 + (targetPos[1] - currentPos[1])**2)**.5
        
        return (rot_amt, distToTarget, targetRotation)

validateScript(scriptStack)

current_balls = (RED, EMPTY, EMPTY)
current_pos_orient = (72, 16.5, -math.pi)

firstPass = []

for command in scriptStack:
    tokens = command.split(" ")

    if tokens[0] == "grab" or tokens[0] == "cycle":
        # determine rotation
        if tokens[0] == "grab":
            ballPos = ballLocations[tokens[1]]
        else:
            ballPos = goalLocations[tokens[1]]

        rotation_amt, dist, newOrient = turnMoveTo((current_pos_orient[0], current_pos_orient[1]), current_pos_orient[2], ballPos)
        
        firstPass.append(f"turn {round(rotation_amt, 3)}")
        firstPass.append(f"drive {round(dist, 3)}")
        if tokens[0] == "grab":
            firstPass.append(f"intake")
        else:
            firstPass.append(f"cycle")

        current_pos_orient = (ballPos[0], ballPos[1], newOrient)
        current_balls = stateAppend(current_balls, RED)
    elif tokens[0] == "eject" and tokens[1] == "SAFE":
        firstPass.append(f"eject SAFE")
    elif tokens[0] == "park":
        distToGoals = list(map(lambda x: ((x[0] - current_pos_orient[0])**2 + (x[1] - current_pos_orient[1])**2)**.5, goalLocations.values()))

        if min(distToGoals) < 24:
            firstPass.append(f"dive {round(24 - min(distToGoals), 3)}")
    else:
        firstPass.append(f"not_implemented {tokens}")

print(firstPass)

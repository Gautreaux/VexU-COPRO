import math
import select
import time
import threading
from typing import Final, List, Tuple

_ODOM_M_VALS = [0, 0, 0, 0]
_ODOM_D_VALUE = 1
_ODOM_NOW_POS = [0,0,0]

_ODOM_CALC_FREQUENCY_S = .05 # 20 times a second 
_ODOM_M_SPACE_TO_C_SPACE = 1 # conversion constant from mouse space to cartesian space

# do not call
#   manages the actual reading
def _ODOM_readLoop(left_mouse, right_mouse):
    _ODOM_M_VALS

    print(f"Odom reader opening mice: {left_mouse}, {right_mouse}")

    with open(left_mouse, 'rb') as lm_file:
        with open(right_mouse, 'rb') as rm_file:
            read_list : Final = [lm_file, rm_file]
            while(True):
                r, _, _ = select.select(read_list, [], [])
                for f in r:
                    k = f.read(3)
                    if f == lm_file:
                        _ODOM_M_VALS[0] += 0 if k[1] == 0 else (1 if k[1] < 128 else - 1)
                        _ODOM_M_VALS[1] += 0 if k[2] == 0 else (1 if k[2] < 128 else - 1)
                    else:
                        _ODOM_M_VALS[2] += 0 if k[1] == 0 else (1 if k[1] < 128 else - 1)
                        _ODOM_M_VALS[3] += 0 if k[2] == 0 else (1 if k[2] < 128 else - 1)

# do not call
#   manages the calcualtions
def _ODOM_calcLoop():
    global _ODOM_M_VALS
    global _ODOM_D_VALUE

    global _ODOM_NOW_POS

    # probably all zeros to begin with
    lastOdomM = _ODOM_M_VALS[:]

    startTime = time.time()

    # periodic loop, will not drift but may drop cycles if things take too long
    while time.sleep((time.time() - startTime) % _ODOM_CALC_FREQUENCY_S) is None:
        nowOdomM = _ODOM_M_VALS[:] # copy so they don't change mid calculation
        odomDelta = list(map(lambda x,y : y - x, lastOdomM, nowOdomM))

        if sum(odomDelta) == 0:
            # no update
            continue

        # we only need deltas now, so can overwrite the old
        lastOdomM = nowOdomM

        # redo names to kepp track of things
        m1x, m1y, m2x, m2y = odomDelta

        # first, find the angular movement
        alpha1 = (m1x**2 + m1y**2)**.5
        alpha2 = (m2x**2 + m2y**2)**.5

        # now start the process of finding circumference traveled
        # find the thetas, notice this is 'backwards' from expected

        #TODO - check the math
        #TODO - what does 0 y change mean?
        theta1 = math.atan(m1x / m1y)
        theta2 = math.atan(m2x / m2y)




def launchOdom(left_mouse : str, right_mouse : str):
    # TODO - something to catch if there are crashing errors
    threading.Thread(target=_ODOM_readLoop, daemon=True, args=(left_mouse, right_mouse)).start()
    threading.Thread(target=_ODOM_calcLoop, daemon=True).start()

# run the calibration procedure
def calibrateOdom() -> None:
    print("ODOM calibration not implmented...")
    
# treat the current position, orientation as the new base
def resetOdom() -> None:
    raise NotImplementedError()

# return the current odometry position, orientation tuple
def getCurrentOdomPosition() -> List[float]:
    global _ODOM_NOW_POS
    

    # copy and return the list position
    return [
        _ODOM_NOW_POS[0] * _ODOM_M_SPACE_TO_C_SPACE,
        _ODOM_NOW_POS[1] * _ODOM_M_SPACE_TO_C_SPACE,
        _ODOM_NOW_POS[2] # is radians, no conversion required
    ]
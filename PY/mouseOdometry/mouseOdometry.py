import math
import select
import time
import threading
from typing import Final, List, Tuple

from ..vexController import vexAction
from .mouseOdometryUtil import determineDFromDeltasRotation, determineMiceByPath, determineTranslationFromDelta
from ..vexMessenger import v_messenger

LEFT_MOUSE_USB_PATH = "1.3"
RIGHT_MOUSE_USB_PATH = "1.2"

_ODOM_M_VALS = [0, 0, 0, 0]
_ODOM_NOW_POS = [0,0,0]

_ODOM_CALC_FREQUENCY_S = .05 # 20 times a second 
_ODOM_M_SPACE_TO_C_SPACE = 1 # conversion constant from mouse space to cartesian space

_ODOM_RUNNING = True
_ODOM_THREADS = [None, None]

# do not call
#   manages the actual reading
def _ODOM_readLoop(left_mouse, right_mouse):
    global _ODOM_M_VALS
    global _ODOM_RUNNING

    print(f"Odom reader opening mice: {left_mouse}, {right_mouse}")

    def asSigned(i):
        if i > 127:
            return -1
        elif i > 0:
            return +1
        else:
            return 0

        # if i > 127:
        #     ii = -(256 - i)
        # else:
        #     ii = i
        # # print(f"{i} --> {ii}")
        # return ii

    with open(left_mouse, 'rb') as lm_file:
        with open(right_mouse, 'rb') as rm_file:
            read_list : Final = [lm_file, rm_file]
            while(_ODOM_RUNNING):
                r, _, _ = select.select(read_list, [], [], 1)
                for f in r:
                    k = f.read(3)
                    if f == lm_file:
                        _ODOM_M_VALS[0] += asSigned(k[1])
                        _ODOM_M_VALS[1] += asSigned(k[2])
                    else:
                        _ODOM_M_VALS[2] += asSigned(k[1])
                        _ODOM_M_VALS[3] += asSigned(k[2])

# do not call
#   manages the calcualtions
def _ODOM_calcLoop(params : Tuple[float]):
    global _ODOM_M_VALS

    global _ODOM_NOW_POS

    global _ODOM_RUNNING

    d_value = params

    # probably all zeros to begin with
    lastOdomM = _ODOM_M_VALS[:]

    startTime = time.time()

    # periodic loop, will not drift but may drop cycles if things take too long
    while _ODOM_RUNNING:
        time.sleep((time.time() - startTime) % _ODOM_CALC_FREQUENCY_S)
        nowOdomM = _ODOM_M_VALS[:] # copy so they don't change mid calculation
        odomDelta = list(map(lambda x,y : y - x, lastOdomM, nowOdomM))

        if sum(odomDelta) == 0:
            # no update
            continue

        # print(f"Odom delta: {odomDelta}")

        # is this true?
        # we only need deltas now, so can overwrite the old
        lastOdomM = nowOdomM

        changeTuple = determineTranslationFromDelta(odomDelta, d_value)

        # print(f"changeTuple: {changeTuple}")

        _ODOM_NOW_POS[2] += changeTuple[2]


def launchReadLoop(left_mouse : str, right_mouse : str) -> threading.Thread:
    global _ODOM_THREADS
    # TODO - something to catch if there are crashing errors
    t = threading.Thread(target=_ODOM_readLoop, daemon=True, args=(left_mouse, right_mouse))
    t.start()
    _ODOM_THREADS[0] = t
    return t


def launchOdomLoop(d_value : float) -> threading.Thread:
    global _ODOM_THREADS
    # TODO - something to catch if there are crashing errors
    t = threading.Thread(target=_ODOM_calcLoop, daemon=True, args = (d_value, ))
    t.start()
    _ODOM_THREADS[1] = t
    return t

def shutdownOdom() -> None:
    global _ODOM_RUNNING
    global _ODOM_THREADS

    _ODOM_RUNNING = False

    for i in range(len(_ODOM_THREADS)):
        if _ODOM_THREADS[i] is not None:
            _ODOM_THREADS[i].join()
            _ODOM_THREADS[i] = None



# run the calibration procedure
def calibrateOdom() -> None:
    print("ODOM calibration not implmented...")
    
# treat the current position, orientation as the new base
def resetOdom() -> None:
    raise NotImplementedError()

# should not be called if the operation loop is running
def getCurrentDeltas() -> List[int]:
    global _ODOM_M_VALS
    return _ODOM_M_VALS[:]

# should not be called if the operation loop is running
def resetCurrentDeltas() -> None:
    global _ODOM_M_VALS
    _ODOM_M_VALS = [0,0,0,0]

# return the current odometry position, orientation tuple
def getCurrentOdomPosition() -> List[float]:
    global _ODOM_NOW_POS
    

    # copy and return the list position
    return [
        _ODOM_NOW_POS[0] * _ODOM_M_SPACE_TO_C_SPACE,
        _ODOM_NOW_POS[1] * _ODOM_M_SPACE_TO_C_SPACE,
        _ODOM_NOW_POS[2] # is radians, no conversion required
    ]

def _doDValueTrial() -> float:
    assert(v_messenger.isConnected())
    print("Starting Trial")
    vexAction.VEX_stop()
    resetCurrentDeltas()
    imu_start = vexAction.VEX_readIMU()
    vexAction.VEX_startRotation(True)
    time.sleep(.3)
    # v_messenger.sendMessage(b"\x00")
    vexAction.VEX_stop()
    imu_delta = vexAction.VEX_readIMU() - imu_start
    print(f"IMU value: {imu_delta}")
    deltas = getCurrentDeltas()
    print(f"Deltas: {deltas}")
    
    d_value = determineDFromDeltasRotation(deltas, imu_delta)

    print(f"D_value: {d_value}")

    return d_value

def resolveDValue(samples = 15) -> float:
    micePaths = determineMiceByPath([LEFT_MOUSE_USB_PATH, RIGHT_MOUSE_USB_PATH])
    print(micePaths)
    launchReadLoop(*micePaths)
    
    v_messenger.connect()
    
    vexAction.VEX_stop()

    l = []
    e_q = 0

    while len(l) < samples:
        try:
            d = _doDValueTrial()
            l.append(d)
            time.sleep(1)
        except ValueError:
            print ("Error occurred")
            e_q += 1

    vexAction.VEX_stop()
    v_messenger.disconnect()

    d_value = sum(l) / len(l)
    print(f"Errors qty: {e_q}")
    print(f"D list: {l}")
    print(f"Resolved d: {d_value}")
    return d_value
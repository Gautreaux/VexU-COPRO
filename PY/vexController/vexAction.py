
import itertools
from typing import Optional, Tuple
from ..vexMessenger import v_messenger

#TODO - proper enum:
COMMAND_ENUM = {
    "stop" : 0,
    "text" : 1,
    "jitter" : 10,
    "startRotationLeft" : 11,
    "startRotationRight" : 12,
    "readIMU" : 32,
    "resetIMU" : 33,
    "goalPos" : 64,
}

class IllegalCommand(Exception):
    pass

def buildSendCommand(key : str, param : Optional[bytes] = None):
    if key not in COMMAND_ENUM:
        raise IllegalCommand(key)

    if param is not None:
        msg = bytes(itertools.chain([COMMAND_ENUM[key]], param))
    else:
        # need to pack into a list so that the result is 1 byte with value
        msg = bytes([COMMAND_ENUM[key]])
    # print(f"Sending: {msg} from {COMMAND_ENUM[key]}:{param}")
    v_messenger.sendMessage(msg=msg)

# stop all robot motion
def VEX_stop() -> None:
    print("Stoping all motion")
    buildSendCommand("stop")

# send and display a message
def VEX_text(s : str) -> None:
    buildSendCommand("text", s.encode("ascii"))

# start move forward and backwards a little distance
#   not implemented
def VEX_jitterStart() -> None:
    raise NotImplementedError()
    print("Beginning jitter")
    buildSendCommand("jitter")

# start a rotation in a direction
def VEX_startRotation(rotate_left : True) -> None:
    if rotate_left:
        buildSendCommand("startRotationLeft")
    else:
        buildSendCommand("startRotationRight")

def VEX_resetIMU() -> None:
    buildSendCommand("resetIMU")

def VEX_readIMU() -> float:
    buildSendCommand("readIMU")
    m = v_messenger.readDataMessageBlocking()
    return float(m)


def VEX_sendGoalTarget(goalPos : Tuple[int, int, int, int]) -> None:
    # print(f"Sending goal pos: {goalPos}")
    buildSendCommand("goalPos", f"{goalPos[0]} {goalPos[1]} {goalPos[2]} {goalPos[3]}".encode("ascii"))
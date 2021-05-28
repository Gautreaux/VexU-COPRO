import itertools
import queue
import serial
from time import sleep
import threading
from typing import Callable, Tuple

from .vexSerialUtil import *


_outboundMessages = queue.Queue()
_connected : bool = False
_vexSerialCallback : Callable[[bytes], None] = None
_echoCallback : Callable[[bytes], None] = None
_connectedEvent = threading.Event()
_disconnectedEvent = threading.Event()
_syncEvents = {}

MAX_RETRIES = 5

HELLO_MSG = b"\x00"
HELLO_ACK_MSG = b"\x01"
GOODBYE_MSG = b"\x09"
GOODBYE_ACK_MSG = b"\x0A"

ECHO_SIG = b"\x02"
ECHO_ACK_SIG = b"\x03"

SYNC_MSG = b"\x04"
SYNC_ACK_MSG = b"\x05"

VALID_CONTROLS = [
    HELLO_MSG, HELLO_ACK_MSG,
    GOODBYE_MSG, GOODBYE_ACK_MSG,
    ECHO_SIG, ECHO_ACK_SIG,
    SYNC_MSG, SYNC_ACK_MSG
]

def _setConnected() -> None:
    global _connected
    global _connectedEvent
    global _disconnectedEvent

    _connected = True
    _connectedEvent.set()
    _disconnectedEvent.clear()

def _setDisconnected() -> None:
    global _connected
    global _connectedEvent
    global _disconnectedEvent

    _connected = False
    _connectedEvent.clear()
    _disconnectedEvent.set()

def sendMessage(msg : bytes) -> None:
    global _outboundMessages
    _outboundMessages.put((msg, False))

def _sendControlMessage(msg : bytes) -> None:
    assert(msg[0:1] in VALID_CONTROLS)
    global _outboundMessages
    _outboundMessages.put((msg, True))

def _messageSender() -> None:
    global _connected
    global _syncEvent
    global _outboundMessages
    while threading.current_thread().is_alive:
        m_pair : Tuple[bytes, bool] = _outboundMessages.get()
        m : bytes = m_pair[0]
        c : bool = m_pair[1]

        _outboundMessages.task_done()

        if v_ser.isOpen() is False:
            print("v_ser closed unexpectedly...")
            continue

        if c is True:
            if m is HELLO_MSG:
                print("VexSerial:Sending hello")
            elif m is HELLO_ACK_MSG:
                print("VexSerial:Sending hello ack")
            elif _connected and m is GOODBYE_MSG:
                print("VexSerial:Sending goodbye")
            elif _connected and m is GOODBYE_ACK_MSG:
                print("VexSerial:Sending goodbye ack")

            t = b"\x00" + m
            print(f"VexSerial::sending control: {t}")
            v_ser.write(b"\x00" + m)

            if m is HELLO_ACK_MSG:
                _setConnected()
            elif _connected and m is GOODBYE_ACK_MSG:
                _setDisconnected()

            continue

        if _connected is False:
            continue

        if len(m) == 0:
            print("WARNING: attempted sending 0-len message")
            continue

        print(f"VexSerial:Sending msg: ({len(m)}) {m}")

        offset = 0
        while offset < len(m):
            thisSize = min(255, len(m) - offset)
            print(f"  VexSerial:Sending: {bytes(itertools.chain([thisSize], m[offset:(offset+thisSize)]))}")
            v_ser.write(bytes(itertools.chain([thisSize], m[offset:(offset+thisSize)])))
            offset += thisSize

def _processEcho() -> None:
    len = v_ser.read()
    assert(len > 0 and len <= 253)
    _sendControlMessage(bytes(itertools.chain([*ECHO_ACK_SIG, len], v_ser.read(len))))

def _processEchoAck() -> None:
    global _echoCallback
    len = v_ser.read()[0]
    msg = v_ser.read(len)
    if _echoCallback is not None:
        _echoCallback(msg)

def _processSync() -> None:
    val = v_ser.read()
    _sendControlMessage(SYNC_ACK_MSG + val)

def _processSyncAck() -> None:
    global _syncEvents
    val = v_ser.read()
    print(f"VexSerial:got sync ack value: {val}")
    if val in _syncEvents:
        _syncEvents[val].set()
    else:
        print(f"VexSerial::Warning got syn ack without matching sync: {val}")

def _messageReceiver() -> None:
    global _connected
    global _syncEvent
    global _vexSerialCallback
    while threading.current_thread().is_alive:  
        b = v_ser.read()
        if b[0] == 0:
            # control sequence
            bb = v_ser.read()
            if bb == HELLO_MSG:
                print("VexSerial:Received hello")
                _sendControlMessage(HELLO_ACK_MSG)
                _setConnected()
            elif bb == HELLO_ACK_MSG:
                print("VexSerial:Received hello ack")
                _setConnected()
            elif bb == GOODBYE_MSG:
                print("VexSerial:Received goodbye")
                _sendControlMessage(GOODBYE_ACK_MSG)
                _setDisconnected()
            elif bb == GOODBYE_ACK_MSG:
                print("VexSerial:Received goodbye ack")
                _setDisconnected()
            elif bb == SYNC_MSG:
                _processSync()
            elif bb == SYNC_ACK_MSG:
                _processSyncAck()
            elif bb == ECHO_SIG:
                _processEcho()
            elif bb == ECHO_ACK_SIG:
                _processEchoAck()
            else:
                print(f"Unrecognized control cod: {bb}")
            continue
        
        print(f"Reading message of length: {b[0]}")
        bb = v_ser.read(b[0])
        if len(bb) != b[0]:
            print(f"WARNING: expected message of length {b[0]} got length {len(bb)} instead")
        if _vexSerialCallback is not None:
            _vexSerialCallback(bb)

def VexSerialDefaultCallback(m : bytes):
    print(m, flush=True)

def setVexSerialCallback(func : Callable[[bytes], None]) -> None:
    global _vexSerialCallback
    _vexSerialCallback = func

def _setEchoCallback(func : Callable[[bytes], None]) -> None:
    global _echoCallback
    _echoCallback = func

def clearVexSerialCallback():
    global _vexSerialCallback
    _vexSerialCallback = None

def VexSerialWaitForConnection():
    global _connectedEvent
    _connectedEvent.wait()

def VexSerialWaitForDisconnection():
    global _disconnectedEvent
    _disconnectedEvent.wait()

def VexSerialTeardown():
    global _connected
    print("VexSerial::beginning teardown")
    if _connected:
        # might be a race condition in here so thats fun
        _read_thread.is_alive = False
        _send_thread.is_alive = False
        _sendControlMessage(GOODBYE_MSG)
        _send_thread.join()
        _read_thread.join()

def VexSerialWaitStreamSync(value : bytes):
    assert(len(value) == 1)
    global _syncEvents

    if value not in _syncEvents:
        _syncEvents[value] = threading.Event()
        _syncEvents[value].clear()
        _sendControlMessage(SYNC_MSG + value)
        _syncEvents[value].wait()
        _syncEvents.pop(value)
    else:
        raise ValueError("Already pending sync with provided value")

retryCtr = 0
while retryCtr < MAX_RETRIES:
    try:
        port = getVexComPort()
        print(f"Resolved vex com port to: {port}")
        v_ser : serial = serial.Serial(port=port, baudrate=115200)
        retryCtr = None
        _setDisconnected()
        print("Successfully opened serial port...")
        break
    except DeviceResolutionFailed as e:
        print(f"Failed to resolve vex device: {e}")
        sleep(.5)
        retryCtr += 1

if retryCtr is not None:
    print(f"Failed to resolve vex device after {MAX_RETRIES} attempts")
    # TODO - shouldn't exit, but should do something else
    #   probably launch everything, but put the retry into another co-ro
    raise DeviceResolutionFailed("ERROR TOO MANY RETRIES")

setVexSerialCallback(VexSerialDefaultCallback)
_echoCallback = None

_read_thread = threading.Thread(target=_messageReceiver)
_send_thread = threading.Thread(target=_messageSender)
_read_thread.start()
_send_thread.start()
_sendControlMessage(HELLO_MSG)
# waitForConnection()
# # sleep(4)
# # sendMessage(b"test")
# # sleep(4)
# # sendMessage(b"ending?")


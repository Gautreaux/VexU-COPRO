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
_connectedEvent = threading.Event()
_disconnectedEvent = threading.Event()

MAX_RETRIES = 5

HELLO_MSG = b"\x00"
HELLO_ACK_MSG = b"\x01"
GOODBYE_MSG = b"\x09"
GOODBYE_ACK_MSG = b"\x0A"

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
    assert(msg in [HELLO_MSG, HELLO_ACK_MSG, GOODBYE_MSG, GOODBYE_ACK_MSG])
    global _outboundMessages
    _outboundMessages.put((msg, True))

def _messageSender() -> None:
    global _connected
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
                v_ser.write(b"\x00" + m)
            elif m is HELLO_ACK_MSG:
                print("VexSerial:Sending hello ack")
                v_ser.write(b"\x00" + m)
                _setConnected()
            elif _connected and m is GOODBYE_MSG:
                print("VexSerial:Sending goodbye")
                v_ser.write(b"\x00" + m)
            elif _connected and m is GOODBYE_ACK_MSG:
                print("VexSerial:Sending goodbye ack")
                v_ser.write(b"\x00" + m)
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

def _messageReceiver() -> None:
    global _connected
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
    # might be a race condition in here so thats fun
    _read_thread.is_alive = False
    _send_thread.is_alive = False
    _sendControlMessage(GOODBYE_MSG)
    _send_thread.join()
    _read_thread.join()

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


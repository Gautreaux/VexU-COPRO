import itertools
from threading import TIMEOUT_MAX
import threading
from typing import Optional
import serial
from time import sleep
import queue

import SerialProxy
from vexSerialUtil import (
    DeviceResolutionFailed, MessageTooLong,
    getVexComPort,
    MAX_MESSAGE_LEN, ILLEGAL_CHAR
)


MAX_CONNECTION_RETRIES = 1

_v_ser_serial_object = None
_SEND_RCV_RUNNING : bool = True
_SENDING_Q = queue.Queue()
_RECEIVE_Q = queue.Queue()

def _deserializeGenerator(msg : bytes):
    i = iter(msg)
    nextSignal = next(i)
    nextSignal-=1
    while True:
        try:
            c = next(i)
            if nextSignal == 0:
                nextSignal = c
                if c == 0:
                    return
                yield ord(ILLEGAL_CHAR)
            else:
                yield c
            nextSignal -= 1
        except StopIteration:
            raise Exception("Unexpected stop iteration...")

# msg should include the final null character
#   returned message will not
def _deserializeMsg(msg : bytes) -> bytes:
    return bytes(_deserializeGenerator(msg))

def _serializeGenerator(msg : bytes):
    plist = []
    ctr = 0
    for b in msg:
        if b == ord(ILLEGAL_CHAR):
            plist.append(ctr+1)
            ctr = 0
        else:
            ctr += 1

    plist.append(ctr+1)
    
    it = iter(msg)

    try:
        for i in range(len(plist)):
            e = plist[i]
            yield(e)
            for _ in range(e-1):
                yield next(it)
            if i != (len(plist) - 1):
                next(it)
        yield 0
    except StopIteration:
        raise Exception("Unexpected stop iteration...")

# returned message will include additional null terminator
def _serializeMsg(msg : bytes) -> bytes:
    return bytes(_serializeGenerator(msg))

def _VexSerialSender():
    global _v_ser_serial_object
    while _SEND_RCV_RUNNING:
        m = _SENDING_Q.get()
        # print(f"Sending: {_serializeMsg(m)}")
        _v_ser_serial_object.write(_serializeMsg(m))
        _v_ser_serial_object.flush()

def _VexSerialReceiver():
    global _v_ser_serial_object
    while _SEND_RCV_RUNNING:
        chunks = []

        c = _v_ser_serial_object.read()
        chunks.append(bytes(c))
        c = int(chunks[-1][-1])
        while c != 0:
            chunks.append(_v_ser_serial_object.read(size=c))
        # print(f"Received chunks: {chunks}")
        # print(f"Received binary: {bytes(itertools.chain.from_iterable(chunks))}")
        _RECEIVE_Q.put(_deserializeMsg(itertools.chain.from_iterable(chunks)))

class NoSerialConnectionException(Exception):
    pass

class VexSerial():

    def __init__(self) -> None:
        global _v_ser_serial_object
        port = None

        retryCtr = 0
        while retryCtr < MAX_CONNECTION_RETRIES:
            try:
                port = getVexComPort()
                break
            except DeviceResolutionFailed as e:
                print(f"Failed to resolve vex device: {e}")
                sleep(.5)
                retryCtr += 1

                if(retryCtr >= MAX_CONNECTION_RETRIES):
                    print(f"Failed to resolve vex device after {retryCtr} attempts")
                    # print(f"  Will use serial proxy")
                    # _v_ser_serial_object = SerialProxy()
                    # break
                    raise NoSerialConnectionException()


        if port:
            print(f"Resolved vex com port to: {port}")
            _v_ser_serial_object = serial.Serial(port=port, baudrate=115200)
            retryCtr = None
            print("Successfully opened serial port...")

        self._sendThread = threading.Thread(target=_VexSerialSender, daemon=True)
        self._recvThread = threading.Thread(target=_VexSerialReceiver, daemon=True)

        self._sendThread.start()
        self._recvThread.start()
    
    def __del__(self):
        self.teardown()

    def sendMessage(self, msg : bytes) -> None:
        if len(msg) > MAX_MESSAGE_LEN:
            raise MessageTooLong()
        _SENDING_Q.put(msg)

    def receiveMessage(self, timeout_s = TIMEOUT_MAX) -> Optional[bytes]:
        try:
            m = _RECEIVE_Q.get(block=True, timeout=timeout_s)
            return m
        except queue.Empty:
            return None
    
    def receiveMessageIfAvailable(self) -> Optional[bytes]:
        try:
            m = _RECEIVE_Q.get_nowait()
            return m
        except queue.Empty:
            return None

    def teardown(self):
        #attempt a clean exit
        _SEND_RCV_RUNNING = False
from enum import Enum, unique
import itertools
import time
from threading import TIMEOUT_MAX
from typing import Callable, Optional
from ..vexSerial import v_ser

class VexMessenger():

    class _MessageHeader():
        def __init__(self, len : int, csum : int, msgID : int, msgType : int) -> None:
            self.len = len
            self.csum = csum
            self.msgID = msgID
            self.msgType = msgType

        def iterate(self):
            yield self.len
            yield self.csum
            yield self.msgID
            yield self.msgType

    # @unique
    # class _MessageTypes(Enum):
    #     MESSAGE_TYPE_DATA = 0,
    #     MESSAGE_TYPE_HELLO = 1,
    #     MESSAGE_TYPE_HELLO_ACK = 2,
    #     MESSAGE_TYPE_GOODBYE = 3,
    #     MESSAGE_TYPE_GOODBYE_ACK = 4,
    #     MESSAGE_TYPE_ECHO = 5,
    #     MESSAGE_TYPE_ECHO_ACK = 6

    class _Message():
        def __init__(self, bytesLike : bytes, msgType : int = 0) -> None:
            self.header = VexMessenger._MessageHeader(len(bytesLike) + 4, 0, 0, msgType)
            self.data = bytesLike

        def as_bytes(self) -> bytes:
            return bytes(itertools.chain(self.header.iterate(), self.data))

    class UnexpectedDisconnection(Exception):
        pass
    
    def __init__(self) -> None:
        self._is_connected = False
        self._echoAckCallback = None
    
    def __del__(self) -> None:
        if(self._is_connected):
            self.try_disconnect(0.2)

    def _send_message(self, msg : 'VexMessenger._Message') -> None:
        v_ser.sendMessage(msg.as_bytes())
    
    # returns None if message could not be recieved in time 
    def _receive_message(self, timeout_s : float = TIMEOUT_MAX) -> Optional['VexMessenger._Message']:
        msg = v_ser.receiveMessage(timeout_s)
        # print(f"Recieved binary: {msg}")
        if msg is None:
            return None
        else:
            # return VexMessenger._Message(msg[4:], VexMessenger._MessageTypes(msg[3]))
            return VexMessenger._Message(msg[4:], msg[3])

    def _try_cycle(self, timeout_s : float, isConnect : bool):
        outType = 1 if isConnect else 3
        inType = 2 if isConnect else 4

        out_message : 'VexMessenger._Message' = VexMessenger._Message(bytes(), outType)

        timeRemaining = timeout_s
        startTime = time.time()

        while self._is_connected != isConnect:
            self._send_message(out_message) # potentially a little spammy
            m : 'VexMessenger._Message'= self._receive_message(timeRemaining)

            if m is None:
                # timeout occurred
                return False

            if m.header.msgType == inType:
                self._is_connected = isConnect
                return True
            elif m.header.msgType == outType:
                out_message.header.msgType = inType
                self._send_message(out_message)
                self._is_connected = isConnect
                return True
            else:
                timeRemaining = timeout_s - (time.time() - startTime)
                if(timeRemaining < 0):
                    return False

    def _handle_control(self, msg : 'VexMessenger._Message'):
        if msg.header.msgType == 1:
            # HELLO
            msg.header.msgType = 2
            self._send_message(msg)
            self._is_connected = True
        elif msg.header.msgType == 2:
            # HELLO_ACK
            self._is_connected = True
        elif msg.header.msgType == 3:
            # GOODBYE
            msg.header.msgType = 4
            self._send_message(msg)
            self._is_connected = False
        elif msg.header.msgType == 4:
            # GOODBYE_ACK
            self._is_connected = False
        elif msg.header.msgType == 5:
            # ECHO
            msg.header.msgType = 6
            self._send_message(msg)
        elif msg.header.msgType == 6:
            # ECHO_ACK
            if self._echoAckCallback is not None:
                self._echoAckCallback(msg)
            pass
        else:
            raise ValueError(f"Illegal control code: {msg.header.msgType}")

    def isConnected(self) -> bool:
        return self._is_connected
    
    # returns true if a connection is established
    #   destroys all pending messages
    def try_connect(self, timeout_s : float = 1.0) -> bool:
        return self._try_cycle(timeout_s, True)

    # returns true if a connection is disconnected cleanly
    #   destroys all pending messages
    def try_disconnect(self, timeout_s : float = 1.0) -> bool:
        return self._try_cycle(timeout_s, False)

    # blocking until connected
    def connect(self):
        self.try_connect(TIMEOUT_MAX)
    
    # blocking until disconnected 
    #   destroys all pending messages
    def disconnect(self):
        self.try_disconnect(TIMEOUT_MAX)

    # read messages until a data message
    #  returns true if a message was successfully read
    #  returns false if a disconnect occurs before timeout
    #  returns false if a timeout occurs before the next data message
    #  immediately returns false if disconnected
    def readDataMessage(self, timeout_s : float) -> Optional[bytes] :
        timeRemaining = timeout_s
        startTime = time.time()

        while(self._is_connected):
            m : VexMessenger._Message = self._receive_message(timeRemaining)

            if m is None:
                # Timeout occurred
                return False
            elif m.header.msgType == 0:
                return m.data
            else:
                self._handle_control(m)
                timeRemaining = timeout_s - (time.time() - startTime)
                if timeRemaining < 0:
                    return False
        return False

    # read messages until a data message
    #  will throw UnexpectedDisconnection if disconnected data message
    #    or if called while disconnected
    def readDataMessageBlocking(self) -> bytes :
        m = self.readDataMessage(TIMEOUT_MAX)
        if m is None:
            raise VexMessenger.UnexpectedDisconnection()
        return m

    def readIfAvailable(self) -> Optional[bytes]:
        return self.readDataMessage(0)

    # send a data message to the other side
    #   may throw UnexpectedDisconnection if disconnected
    def sendMessage(self, msg : bytes):
        if self._is_connected == False:
            raise VexMessenger.UnexpectedDisconnection()
        self._send_message(VexMessenger._Message(msg, msgType=0))

    def setEchoAckCallback(self, func_ptr : Callable[['VexMessenger._Message'], None]) -> None:
        self._echoAckCallback = func_ptr

    def clearEchoAckCallback(self):
        self._echoAckCallback = None

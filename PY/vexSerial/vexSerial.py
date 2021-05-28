import itertools
import threading
from typing import ByteString, Callable
import serial

class VexSerial:
    def __init__(
        self,
        port : str,
        readCallback : Callable[['VexSerial', ByteString], None],
        baudrate : int = 115200,
    ) -> None:
        try:
            self._serialport = serial.Serial(port=port, baudrate=baudrate)
            self._callback = readCallback

            self._readThreadOK = True
            self._readThread = threading.Thread(target=self._recvData)
            self._readThread.start()

            self._ok = True

            self._sendHello()
        except Exception as e:
            self._readThread = None
            self._serialport = None
            self._ok = False

    def __bool__(self):
        return self._ok
    
    def __del__(self):
        self._stopRecv()
        if self._serialport is not None:
            self._serialport.close()
            self._serialport = None

    def _recvData(self):
        while self._readThreadOK:
            b = self._serialport.read()
            if b[0] == 0:
                # control sequence
                self._recvControl()
                continue
            bb = self._serialport.read(b[0])
            if len(bb) != b[0]:
                print(f"WARNING: expected message of length {b[0]} got length {len(bb)} instead")
            self._callback(self, bb)
        self._ok = False

    def _recvControl(self):
        b = self._serialport.read()
        if b[0] == 0:
            print(f"Received hello from serial port")
            self._sendHelloAck()
        elif b[0] == 1:
            print(f"Hello, ack'ed")
        elif b[0] == 9:
            print(f"Received goodbye from serial port")
            self.__sendGoodbyeAck()
            self._readThreadOK = False
        elif b[0] == 10:
            print(f"Goodbye, ack'ed")

    def _stopRecv(self):
        if self._readThread is not None:
            self.__sendGoodbye()
            self._readThreadOK = False
            self._readThread.join()
            self._readThread = None
            self._ok = False

    def _sendHello(self):
        self._serialport.write(b"\x00\x00")
    
    def __sendGoodbye(self):
        self._serialport.write(b"\x00\x01")

    def _sendHelloAck(self):
        self._serialport.write(b"\x00\x09")
    
    def __sendGoodbyeAck(self):
        self._serialport.write(b"\x00\x0A")

    def _sendData(self, data : ByteString):
        if len(data) == 0:
            self.write(b"\x00")

        offset = 0
        while offset < len(data):
            thisDist = min(255, len(data) - offset)
            self._serialport.write(bytes(itertools.chain([thisDist], data[offset:(offset+thisDist)])))
            offset += thisDist

    def sendData(self, data):
        if len(data) == 0:
            print("WARNING: user attempted empty message")
            return
        self._sendData(data)
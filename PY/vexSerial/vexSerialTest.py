import itertools
from time import sleep
from .vexSerialUtil import getVexComPort
from .vexSerial import VexSerial

def testAllDatagrams():
    v = itertools.chain.from_iterable(itertools.product(list(range(256)), repeat=2))
    b = bytes(v)

    print(f"Generated test data of {len(b)} bytes")

    responseList = []
    
    def cb(_ : VexSerial, b : bytes):
        responseList.append(b)

    vser = VexSerial(getVexComPort(), cb)
    vser.sendData(b)

    print(f"Done sending serial data")

    for _ in range(50):
        # want to make sure everything is received and I don't want to use asyncio
        sleep(0.2)

    r = bytes(itertools.chain.from_iterable(responseList))

    assert(b == r)

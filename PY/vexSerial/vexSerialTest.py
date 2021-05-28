import itertools
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

    r = bytes(itertools.chain.from_iterable(responseList))

    assert(b == r)

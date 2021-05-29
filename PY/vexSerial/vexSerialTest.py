import itertools
from . import ECHO_SIG, VexSerialWaitStreamSync, _sendControlMessage, setEchoCallback
import random
from time import sleep

DEFAULT_TEST_SIZE = 10*(2**20) # 10MB

def testRandomDatagram(testSize : int = DEFAULT_TEST_SIZE):
    print(f"Testing random data of size {testSize}")
    myBytes = random.randbytes(testSize)

    resBytes = []
    def echoCallback(rBytes : bytes) -> None:
        resBytes.append(rBytes)
    setEchoCallback(echoCallback)

    exRounds = (len(myBytes) + 252) // 253
    print(f"Expected rounds = {exRounds}")
    roundCounter = 0

    offset = 0
    while offset < len(myBytes):
        thisSize = min(253, len(myBytes) - offset)
        _sendControlMessage(bytes(itertools.chain([*ECHO_SIG, thisSize], myBytes[offset:(offset+thisSize)])))
        sleep(1)
        offset += thisSize
        roundCounter += 1
        if(roundCounter % 250 == 0):
            print(f"{roundCounter}/{exRounds}, responses received = {len(resBytes)}")
    
    print(f"Done Sending, syncing")
    VexSerialWaitStreamSync(random.randbytes(1))
    print(f"Done syncing")

    response = bytes(itertools.chain.from_iterable(resBytes))

    print(f"Result: {response == myBytes}")
    assert(response == myBytes)
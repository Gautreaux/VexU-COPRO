from .vexSerial import _serializeMsg, _deserializeMsg, _serializeGenerator, _deserializeGenerator

DEFAULT_TEST_SIZE = 10*(2**20) # 10MB

# TODO - rework this mess
# def testRandomDatagram(testSize : int = DEFAULT_TEST_SIZE):
#     print(f"Testing random data of size {testSize}")
#     myBytes = random.randbytes(testSize)

#     resBytes = []
#     def echoCallback(rBytes : bytes) -> None:
#         resBytes.append(rBytes)
#     setEchoCallback(echoCallback)

#     exRounds = (len(myBytes) + 252) // 253
#     print(f"Expected rounds = {exRounds}")
#     roundCounter = 0

#     offset = 0
#     while offset < len(myBytes):
#         thisSize = min(253, len(myBytes) - offset)
#         _sendControlMessage(bytes(itertools.chain([*ECHO_SIG, thisSize], myBytes[offset:(offset+thisSize)])))
#         sleep(1)
#         offset += thisSize
#         roundCounter += 1
#         if(roundCounter % 250 == 0):
#             print(f"{roundCounter}/{exRounds}, responses received = {len(resBytes)}")
    
#     print(f"Done Sending, syncing")
#     VexSerialWaitStreamSync(random.randbytes(1))
#     print(f"Done syncing")

#     response = bytes(itertools.chain.from_iterable(resBytes))

#     print(f"Result: {response == myBytes}")
#     assert(response == myBytes)

def testSerializeDeserialize():
    msgSerialPairs = [
        (b"apple", b"\x02a\x01\x03le\x00"),
        (b"penguin", b"\x01\x07enguin\x00"),
        (b"lap", b"\x03la\x01\x00"),
        (b"pppp", b"\x01\x01\x01\x01\x01\x00"),
        (b"p", b"\x01\x01\x00"),
        (b"nothing", b"\x08nothing\x00"),
        (b"u", b"\x02u\x00"),
    ]

    for msg,ser in msgSerialPairs:
        print(list(_deserializeGenerator(ser)))
        print(_deserializeMsg(ser))
        assert(msg == _deserializeMsg(ser))
        
    print("Deserialize ok")
        
    for msg,ser in msgSerialPairs:
        print(list(_serializeGenerator(msg)))
        print(_serializeMsg(msg))
        assert(ser == _serializeMsg(msg))
    
    print("Serialize ok")
    print("SER/DSER PASSING!!!")
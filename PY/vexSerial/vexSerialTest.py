import time
from .vexSerial import (_serializeMsg, _deserializeMsg, _serializeGenerator, _deserializeGenerator)
from . import v_ser
from random import randbytes

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

def wordTest():
    s = "apple lap penguin Antimodernistic bloomiest unculture squamosely prejudicial imbrown bimbo consonantize. Focalize danny arteriosclerosis proboxing hyperemotivity faro pompous haeres. Nonusurious euphonicalness pseudocatholically diverging unvenerated catania disgrace keratose. Philomela pettishness multispermous leukopenia effacer canephora kabala hopsack. Wavell clumsily barolo endocrinologic advanced moseyed eiffel narcomaniacal. Lulu pairle interlibelled enate bottleneck gossamery penanceless blackamoor. Sudor kisumu ratatouille jumbled jacinth findlay saltness kean. Sunlit surabaya holdable rowen moonlight pokier patchstand peppery. Discontinue boronic authoriser repellency prouniversity homothermism virginis sacristan. Proud wran cetological skein kuban uncatastrophically innervate subtriangularity. Lounging unchaste unapostrophized stouthearted cleanthes shrilly harebell berme. Nonguaranty nonresilience zipper auscultate shoji unhelved sinful mazedly. Lah electrotactic valdosta overactivate clois unpunctilious disguising mistiest. Granite tanana courses domorphous thug isn''t mir sandstone. Pontificals loxodromics dingily interwreathed unsystematising mismated immeasurable eudaemonistical. Geographical prenotifying dorsiventrality overproved gnawable radicalness charleton outdriven. Uncentralized estivation cartwheel nemesis noncrystallized mealies aerosphere intercommunity. Burrstone revivable pharmaceutical undersplicing inconsiderably prepolitic sutra cartelism. Oophorectomize zool calliste describing kingman jamaica wattenscheid navasota. Blusterous forbye lobscourse frugality overarch quinidine hadhramaut furmenty."
    s = s.replace('.', '').split(" ")

    for k in s:
        k = bytes(k, encoding="ascii")
        v_ser.sendMessage(k)
        m = v_ser.receiveMessage()
        if(m != k):
            print(f"Word: {k}")
            print(f"Sent: {k}")
            print(f"Recv: {m}")
            assert(m == k)

def bytesTest():
    TOTAL_BYTES = 8*(2**20) # 8 MB
    BYTES_PER_MESSAGE = 100
    
    rounds = (TOTAL_BYTES // BYTES_PER_MESSAGE) + 1

    nextReport = 0
    reportFreq = .005 # every 0.5%

    startTime = time.time() 

    for i in range(rounds):
        k = randbytes(BYTES_PER_MESSAGE)
        v_ser.sendMessage(k)
        m = v_ser.receiveMessage()
        if(m != k):
            print(f"Sent: {k}")
            print(f"Recv: {m}")
            assert(m == k)

        p = i / rounds
        if p > nextReport:
            nextReport += reportFreq
            elapsed = time.time() - startTime
            print(f"{round(p*BYTES_PER_MESSAGE,2)}% complete, {BYTES_PER_MESSAGE*i} bytes, {round(elapsed, 2)} sec, {round(BYTES_PER_MESSAGE*i/(elapsed * 1024), 4)} Kbps")
        
    elapsed = time.time() - startTime
    print(f"Sent {round((rounds * BYTES_PER_MESSAGE)/(1024), 2)} Kilobytes without error in {round(elapsed,2)} sec, approx speed: {round((rounds*BYTES_PER_MESSAGE)/(1024*elapsed), 4)} Kbps")
from enum import Enum
import RPi.GPIO as GPIO

RED_1   =  2
GREEN_1 =  3
BLUE_1  =  4

RED_2   = 27
GREEN_2 = 22
BLUE_2  = 10

RED_3   =  9
GREEN_3 = 11
BLUE_3  =  5

RED_4   = 13
GREEN_4 = 19
BLUE_4  = 26

GPIO.setmode(GPIO.BCM)

class RGB(Enum):
    RED = (1,0,0)
    GREEN = (0,1,0)
    BLUE = (0,0,1)

class Pattern(Enum):
    OFF = [0]
    QUARTER_BLINK = [0,0,0,1]
    HALF_BLINK = [0,1]
    THREE_QUARTER_BLINK = [0,1,1,1]
    ON = [1]

    HEARTBEAT = [0,1,0,0,0,1]

class RGB_LED():

    def __init__(self, red_pin, green_pin, blue_pin):
        self.pins = [red_pin, green_pin, blue_pin]
        for p in self.pins:
            GPIO.setup(p, GPIO.OUT)
            GPIO.output(p, GPIO.LOW)
        self.op_code = ""
        self.color = (0,0,0)
        self.changed_op = False
        self.iter = None

    def update(self):
        if self.changed_op:
            self.iter = None
            self.changed_op = False

        if self.iter == None:
            self.iter = iter(self.op_code)

        if not self.op_code:
            for p in self.pins:
                GPIO.output(p, GPIO.LOW)
            return
        
        try:
            i = next(self.iter)
        except StopIteration:
            self.iter = iter(self.op_code)
            try:
                i = next(self.iter)
            except StopIteration:
                print(f"LED CRASHED: BAD OP: `{self.op_code}`")
                self.op_code = ""
                return self.update()
        
        for bit, pin in zip(self.color, self.pins):
            GPIO.output(pin, GPIO.HIGH if bit and i else GPIO.LOW)

    def setColor(self, color):
        self.color = color if not isinstance(color, Enum) else color.value

        for bit, pin in zip(self.color, self.pins):
            GPIO.output(pin, GPIO.HIGH if bit else GPIO.LOW)
    
    def setOp(self, op):
        self.op_code = op if not isinstance(op, Enum) else op.value
        self.changed_op = True


def getLEDs():
    return [
     RGB_LED(RED_1, GREEN_1, BLUE_1),
     RGB_LED(RED_2, GREEN_2, BLUE_2),
     RGB_LED(RED_3, GREEN_3, BLUE_3),
     RGB_LED(RED_4, GREEN_4, BLUE_4),
    ]

if __name__ == "__main__":
    print("Running rgb LED locally")
    leds = getLEDs()

    from time import sleep

    def rainbowGen():
        while True:
            yield RGB.RED
            yield RGB.GREEN
            yield RGB.BLUE

    leds[0].setColor(RGB.RED)
    leds[1].setColor(RGB.BLUE)
    leds[2].setColor(RGB.GREEN)

    leds[0].setOp(Pattern.HALF_BLINK)
    leds[1].setOp(Pattern.HEARTBEAT)
    leds[2].setOp([1,1,1,0])
    
    try:
        while True:
            for led in leds:
                led.update()
            print("HEARTBEAT")
            sleep(1)
    finally:
        GPIO.cleanup()

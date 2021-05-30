from .vexSerial import v_ser
from .vexSerial.vexSerialTest import wordTest, bytesTest

from time import sleep

def main():

    wordTest()
    print("Word test done")
    bytesTest()
    print()
    print("Byte test done")
    print("Test concluded successfully")

if __name__ == "__main__":
    # this file was run directly
    print("Running directly (no init) (this <probably> wont matter?)")
    main()
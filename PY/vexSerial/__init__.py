from .vexSerial import VexSerial

# v_ser : VexSerial = VexSerial()
v_ser = None

def newInit(void):
    raise Exception("VexSerial is singleton, cannot create new instances")
VexSerial.__init__ = newInit


from .vexMessenger import VexMessenger

# v_messenger : VexMessenger = VexMessenger()
v_messenger = None

def newInit(void):
    raise Exception("VexMessenger is singleton, cannot create new instances")
VexMessenger.__init__ = newInit
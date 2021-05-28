from typing import Optional, List
import serial.tools.list_ports
from serial.tools.list_ports_common import ListPortInfo

class DeviceResolutionFailed(Exception):
    pass

# prints the port info or headers on none
def printPortInfo(port : Optional[ListPortInfo]) -> None:
    if port is None:
        print("device vendor_id product_id product_name manufacturer interface hardware_id description")
    else:
        print(f"{port.device} {port.vid} {port.pid} \"{port.product}\" \"{port.manufacturer}\" \"{port.interface}\" \"{port.hwid}\" \"{port.description}\"")

# lists all the ports connected
#   pass verbose=true for more details
def listAllPorts(verbose : bool = False) -> None:
    portsList = serial.tools.list_ports.comports()
    print(f"List all ports found {len(portsList)} ports")

    if verbose:
        printPortInfo(None) # print headers
        for port in portsList:
            printPortInfo(port)
    else:
        for port in portsList:
            print(port.device)

# locate the vex com port
#   raises exception on error
def getVexComPort() -> str:
    portsList : List[ListPortInfo] = serial.tools.list_ports.comports()
    print(f"Found {len(portsList)} devices on com ports")

    portsList = [p for p in portsList if 
        p.manufacturer is not None and "VEX" in p.manufacturer and
        p.description is not None and "V5 User Port" in p.description
    ]

    if len(portsList) == 0:
        print("No valid port could be found")
        print("All devices are:")
        listAllPorts(True)
        raise DeviceResolutionFailed("No devices matched filters")
    elif len(portsList) > 1:
        print("Multiple Matching Devices")
        print("Devices are:")
        map(printPortInfo, portsList)
        raise DeviceResolutionFailed("Multiple devices matched filters")
    return portsList[0].device
#include <iostream>
#include <sys/types.h> // for open
#include <sys/stat.h> // for open
#include <fcntl.h> // for open
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <vector>

namespace utils
{
    // set the attrubutes on a serial FD
    int set_interface_attribs(int fd, int speed, int parity);

    // set the blocking status on a serial FD
    int set_blocking(int fd, int should_block);

    // resolve the target port using system calls
    std::string getACMPort(void);

    // do all the setup on the serial port
    int getSetupSerialFD(void);
};
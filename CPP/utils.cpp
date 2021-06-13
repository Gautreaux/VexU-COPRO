#include "utils.h"

namespace utils
{
    int set_interface_attribs(int fd, int speed, int parity)
    {
        struct termios tty;
        if (tcgetattr(fd, &tty) != 0)
        {
            printf("error %d from tcgetattr", errno);
            return -1;
        }

        cfsetospeed(&tty, speed);
        cfsetispeed(&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK; // disable break processing
        tty.c_lflag = 0;        // no signaling chars, no echo,
                                // no canonical processing
        tty.c_oflag = 0;        // no remapping, no delays
        tty.c_cc[VMIN] = 0;     // read doesn't block
        tty.c_cc[VTIME] = 5;    // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);   // ignore modem controls,
                                           // enable reading
        tty.c_cflag &= ~(PARENB | PARODD); // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr(fd, TCSANOW, &tty) != 0)
        {
            printf("error %d from tcsetattr", errno);
            return -1;
        }
        return 0;
    }

    int set_blocking(int fd, int should_block)
    {
        struct termios tty;
        memset(&tty, 0, sizeof tty);
        if (tcgetattr(fd, &tty) != 0)
        {
            printf("error %d from tggetattr", errno);
            return -1;
        }

        tty.c_cc[VMIN] = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5; // 0.5 seconds read timeout

        if (tcsetattr(fd, TCSANOW, &tty) != 0)
        {
            printf("error %d setting term attributes", errno);
            return -1;
        }

        return 0;
    }

    std::string getACMPort(void)
    {
        char outputBuffer[1024];
        memset(outputBuffer, 0, 1024);

        // searching in the serail directory for symbolic links to usb address of vex serial port
        char cmd[] = "ls -l /dev/serial/by-path/ | grep usb-0:1.1.3:1.2";

        FILE* pipe = popen(cmd, "r");

        std::vector<std::string> partials;

        if(!pipe){
            printf("Failed to launch sub-command: %s", cmd);
        }

        try {
            while(fgets(outputBuffer, 1024, pipe) != NULL){
                outputBuffer[strlen(outputBuffer) - 1] = 0;
                partials.emplace_back(outputBuffer);
                memset(outputBuffer, 0, 1024);
            }
        } catch (...){
            printf("Error reading from pipe.\n");
            pclose(pipe);
            return "";
        }
        pclose(pipe);

#ifdef DEBUG
        printf("Raw Partials are:\n");
        for(auto& s : partials){
            printf("  %s\n", s.c_str());
        }
#endif

        std::vector<std::string> raw = std::move(partials);

        for(auto& s : raw){
            char const * const c = s.c_str();
            auto substr = strrchr(c, '/');
            partials.emplace_back(substr+1);
        }

        if(partials.size() < 1){
            printf("No paths could be resolved.\n");
            return "";
        }
        if(partials.size() > 1){
            printf("Multiple paths found, returning first one...");
            for(auto& s : partials){
                printf("  %s\n", s.c_str());
            }
        }
        return "/dev/" + partials[0];
    }

    int getSetupSerialFD(void)
    {

        std::string port = getACMPort();

        if(port == ""){
            return -1;
        }

        printf("Resolved serial port to: %s\n", port.c_str());

        int serialFD = open(port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
        if(serialFD < 0){
            printf("Error %d occurred which opening serial file.\n", errno);
            return -1;
        }

        if(set_interface_attribs(serialFD, B115200, 0) != 0){
            return -1;
        }

        if(set_blocking(serialFD, 1) != 0){
            return -1;
        }

        return serialFD;
    }
}
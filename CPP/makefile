
CXX=g++
CPPFLAGS=
CPPSTD=-std=c++17
CPP_COMP_COM=$(CXX) $(CPPFLAGS) $(CPPSTD) -DNOT_PROS
CC_COMPLIE_NO_LINK_AUTO=$(CPP_COMP_COM) -c -Wall -o $@ $<
CC_COMPILE_LINK_EXE_AUTO=$(CPP_COMP_COM) -Wall -o $@ $< OFiles/*.o

EXECUTABLE=Main

.PHONY: _BUILD_SETUP_ clean

	
$(EXECUTABLE) : main.cpp clean _BUILD_SETUP_ OFiles/vexMessenger.o
	$(CC_COMPILE_LINK_EXE_AUTO)

OFiles/vexSerial.o : ../PROS/src/vexSerial.cpp ../PROS/include/vexSerial.h
	$(CC_COMPLIE_NO_LINK_AUTO)

OFiles/vexMessenger.o : ../PROS/src/vexMessenger.cpp ../PROS/include/vexSerial.h OFiles/vexSerial.o
	$(CC_COMPLIE_NO_LINK_AUTO)

OFiles/ThreadQueue.o : ThreadQueue.h ThreadQueue.cpp
	$(CC_COMPLIE_NO_LINK_AUTO)

clean:
	rm -rf OFiles/*
	rm -f main
	rm -f *.out

_BUILD_SETUP_:
	mkdir -p OFiles

ser_check:
	python3.9 -m serial.tools.miniterm

CXX=g++
CPPFLAGS=
CPPSTD=-std=c++17
CPP_COMP_COM=$(CXX) $(CPPFLAGS) $(CPPSTD) -DNOT_PROS -lpthread -Wall -DDEBUG
CC_COMPLIE_NO_LINK_AUTO=$(CPP_COMP_COM) -c -o $@ $<
CC_COMPILE_LINK_EXE_AUTO=$(CPP_COMP_COM) -o $@ $< OFiles/*.o

EXECUTABLE=Main

.PHONY: _BUILD_SETUP_ clean run

	
$(EXECUTABLE) : main.cpp OFiles/vexMessenger.o
	$(CC_COMPILE_LINK_EXE_AUTO)

OFiles/vexSerial.o : vexSerial.cpp vexSerial.h
	$(CC_COMPLIE_NO_LINK_AUTO)

OFiles/vexMessenger.o : vexMessenger.cpp vexMessenger.h OFiles/vexSerial.o ThreadQueue.h
	$(CC_COMPLIE_NO_LINK_AUTO)

run: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	rm -rf OFiles/*
	rm -f main
	rm -f *.out

_BUILD_SETUP_:
	mkdir -p OFiles
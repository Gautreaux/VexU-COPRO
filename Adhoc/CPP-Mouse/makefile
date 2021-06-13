
CXX=g++
CPPFLAGS=
CPPSTD=-std=c++17
CPP_COMP_COM=$(CXX) $(CPPFLAGS) $(CPPSTD) -DNOT_PROS -lpthread -Wall -DDEBUG -g
CC_COMPLIE_NO_LINK_AUTO=$(CPP_COMP_COM) -c -o $@ $<
CC_COMPILE_LINK_EXE_AUTO=$(CPP_COMP_COM) -o $@ $< OFiles/*.o

EXECUTABLE=Main

.PHONY: _BUILD_SETUP_ clean run

	
$(EXECUTABLE) : main.cpp
	$(CC_COMPILE_LINK_EXE_AUTO)

run: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	rm -rf OFiles/*
	rm -f main
	rm -f *.out

_BUILD_SETUP_:
	mkdir -p OFiles
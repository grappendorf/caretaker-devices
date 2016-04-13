DEVICE_SOURCE=../src/remotecontrol.cpp
SIMLIB_PATH=../../caretaker-device-simulator/cpp
SOURCES=$(SIMLIB_PATH)/*.cpp $(DEVICE_SOURCE) remotecontrol-sim.cpp
HEADERS=$(SIMLIB_PATH)/*.h
GCC_OPTS=-DARDUINO=100 -std=c++0x
LIBS=-lhiredis -l:libevent-2.0.so.5.1.9

sim: $(HEADERS) $(SOURCES)
	g++ -I $(SIMLIB_PATH) -o sim $(GCC_OPTS) $(SOURCES) $(LIBS)

dockerimage: sim
	docker build -t grappendorf/caretaker-device-remotecontrol .

dockerrun: dockerimage
	docker run --name caretaker-remotecontrol -d --net caretaker grappendorf/caretaker-device-remotecontrol -s workspace-caretaker-server -r caretaker-redis remote

clean:
	-docker rmi grappendorf/caretaker-device-remotecontrol
	rm -f sim

DEVICE_SOURCE=../src/switch.cpp
SIMLIB_PATH=../../caretaker-device-simulator/cpp
SOURCES=$(SIMLIB_PATH)/*.cpp $(DEVICE_SOURCE) switch-sim.cpp
HEADERS=$(SIMLIB_PATH)/*.h
GCC_OPTS=-DARDUINO=100 -std=c++0x
LIBS=-lhiredis -l:libevent-2.0.so.5.1.9

sim: $(HEADERS) $(SOURCES)
	g++ -I $(SIMLIB_PATH) -o sim $(GCC_OPTS) $(SOURCES) $(LIBS)

dockerimage: sim
	docker build -t grappendorf/caretaker-device-switch .

dockerrun: dockerimage
	docker run --name caretaker-switch -d --net caretaker grappendorf/caretaker-device-switch -s workspace-caretaker-server -r caretaker-redis switch

clean:
	-docker rmi grappendorf/caretaker-device-switch
	rm -f sim

CPP = g++
CPPFLAGS = -Wall -Werror -O3
INC = `wx-config --cflags` `gphoto2-config --cflags`
LIBS = `gphoto2-config --libs` `wx-config --libs`

all: bin/gtimelapse

src/globals.o: src/globals.cpp 
	$(CPP) $(INC) $(CPPFLAGS) -c src/globals.cpp -o src/globals.o

src/gphotofunctions.o: src/gphotofunctions.cpp
	$(CPP) $(INC) $(CPPFLAGS) -c src/gphotofunctions.cpp -o src/gphotofunctions.o

src/gtimelapse.o: src/gtimelapse.cpp
	$(CPP) $(INC) $(CPPFLAGS) -c src/gtimelapse.cpp -o src/gtimelapse.o

bin/gtimelapse: src/globals.o src/gphotofunctions.o src/gtimelapse.o
	$(CPP) $(INC) $(CPPFLAGS) src/globals.o src/gphotofunctions.o src/gtimelapse.o -o bin/gtimelapse ${LIBS} 

clean:
	rm bin/gtimelapse src/*.o

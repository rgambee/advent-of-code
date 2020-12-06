.PHONY = all clean

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror
ALL_FLAGS = -I ./utils/ -pthread ${CXXFLAGS}

SOURCES = $(wildcard day??/solution??.cpp)
INPUTS = $(wildcard day??/input??.txt)
EXECUTABLES = $(patsubst %.cpp,%.exe,${SOURCES})


all: ${EXECUTABLES}


%.exe: %.cpp
	${CXX} ${ALL_FLAGS} -o $@ $< ./utils/*.cpp


clean:
	rm ${EXECUTABLES}

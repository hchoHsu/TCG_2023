CXXFLAGS = -std=c++11 -O2 -Wall -Wextra

all: testing

.PHONY: testing
testing: src/testing.cpp src/ewn.cpp
	g++ ${CXXFLAGS} $^ -o $@

clean:
	rm -f testing

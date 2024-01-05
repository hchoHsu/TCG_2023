CXXFLAGS = -std=c++11 -O2

all: baseline

baseline: src/main.cpp src/MyAI.cpp
	g++ $(CXXFLAGS) $^ -o $@

clean:
	rm -f baseline

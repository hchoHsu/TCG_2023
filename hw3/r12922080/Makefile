CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -ffast-math
# CXXFLAGS = -std=c++17 -O3
# CXXFLAGS = -std=c++11 -O3 -ffast-math
# CXXFLAGS = -std=c++11 -O3

all: agent

agent: src/main.cpp src/MyAI.cpp src/Agent.cpp
	g++ $(CXXFLAGS) $^ -o $@

test: src/test_main.cpp src/Agent.cpp
	g++ $(CXXFLAGS) $^ -o $@

clean:
	rm -f agent test

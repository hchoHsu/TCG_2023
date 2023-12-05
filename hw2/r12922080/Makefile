# CXXFLAGS = -std=c++11 -O3 -Wall -Wextra -ffast-math
# CXXFLAGS = -std=c++17 -O3
CXXFLAGS = -std=c++11 -O3 -ffast-math
# CXXFLAGS = -std=c++11 -O3

all: agent

agent: src/agent.cpp src/mcts.cpp
	g++ $(CXXFLAGS) $^ -o $@

clean:
	rm -f agent

CFLAGS = -std=c++11 -O2 -Wall -Wextra

all:
	@echo "usage: make verifier"

agent: src/main.cpp src/ewn.cpp
	g++ -std=c++11 -O3 -Wall -Wextra $^ -o $@

verifier: src/verifier.cpp src/ewn.cpp
	g++ ${CFLAGS} $^ -o $@

clean:
	rm -f verifier agent

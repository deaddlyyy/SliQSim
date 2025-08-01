ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

CFLAGS = -I $(ROOT_DIR)/cudd/cudd -I $(ROOT_DIR)/cudd/util -I $(ROOT_DIR)/cudd/
LFLAGS = -static -L $(ROOT_DIR)/cudd/cudd/.libs/ -lcudd -lm -lboost_program_options  -lgmpxx -lgmp

CXX = g++

.PHONY: all

all: 
	cd cudd && $(MAKE)
	$(CXX) src/*.cpp -o SliQSim $(CFLAGS) $(LFLAGS)
	mv SliQSim ../src/

.PHONY: clean

clean:
	cd cudd && $(MAKE) clean
	rm -f SliQSim

CC      =  g++
CFLAGS  = -D __STDC_FORMAT_MACROS -D __STDC_LIMIT_MACROS -std=c++11
#CFLAGS  = -D __STDC_FORMAT_MACROS -D __STDC_LIMIT_MACROS
LDFLAGS = -lboost_program_options -lboost_regex -lz
LIB_DIR = -L/usr/lib64 -L/usr/lib -L$(HOME)/lib
LIB     = 
INC     = -I./include -I$(HOME)/lib/include -I./cb_minisat
MINISAT_OBJS = cb_minisat/build/release/minisat/core/Solver.o  cb_minisat/build/dynamic/minisat/utils/System.o

all: minisat LKH cbTSP

debug: CFLAGS += -g
debug: all

release: CFLAGS += -DNDEBUG
release: all

install: release
	cp cbTSP ~/bin
	cp cbLKH/cbLKH ~/bin

minisat:
	cd cb_minisat &&\
	echo make

LKH:
	cd cbLKH &&\
	echo make &&\
	echo mv LKH cbLKH

obj/formula.o: include/formula.hpp src/formula.cpp
	$(CC) $(CFLAGS) -fPIC $(INC) $(LIB_DIR) -c src/formula.cpp -o obj/formula.o

obj/theories.o: include/theories.hpp src/theories.cpp
	$(CC) $(CFLAGS) -fPIC $(INC) $(LIB_DIR) -c src/theories.cpp -o obj/theories.o

obj/tsp.o: include/tsp.hpp src/tsp.cpp
	$(CC) $(CFLAGS) -fPIC $(INC) $(LIB_DIR) -c src/tsp.cpp -o obj/tsp.o

obj/sattsp.o: include/sattsp.hpp src/sattsp.cpp
	$(CC) $(CFLAGS) -fPIC $(INC) $(LIB_DIR) -c src/sattsp.cpp -o obj/sattsp.o

obj/cbTSP.o: include/main.hpp src/cbTSP.cpp
	$(CC) $(CFLAGS) -fPIC $(INC) $(LIB_DIR) -c src/cbTSP.cpp -o obj/cbTSP.o

cbTSP: minisat obj/formula.o obj/theories.o obj/tsp.o obj/sattsp.o obj/cbTSP.o 
	$(CC) $(CFLAGS) -fPIC $(INC) $(LIB_DIR) $(MINISAT_OBJS) obj/formula.o obj/theories.o obj/tsp.o obj/sattsp.o obj/cbTSP.o -o cbTSP $(LIB) $(LDFLAGS)

clean:
	echo cd cb_minisat && echo make clean
	echo cd cbLKH && echo make clean
	rm -rf *~
	rm -rf */*~
	rm -rf */*/*~
	rm -f obj/*
	rm -f *.so
	rm -f cbTSP
	

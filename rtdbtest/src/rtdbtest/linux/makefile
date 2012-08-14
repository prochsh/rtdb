HOME  =/home/d5000/ems
CCFLAGS = -L$(HOME)/lib -lrtdb_api -I$(HOME)/src/include -I$(HOME)/db_def
prog:read_rtdb.o
%.o: %.cc
	g++  -o $@ $(CCFLAGS) $^


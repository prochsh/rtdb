.SUFFIXES: .o .c .cxx
OBJS=rtdb_test.o
ORACLE=/oracle/app/oracle/product/9.2
INC_DIR = -I. -I$(HOME)/src/include -I$(HOME)/src/include/db_api -I$(HOME)/src/include/msg_api -I$(HOME)/service/src/include  -I$(HOME)/src/include/security -I$(HOME)/src/platform/servicebus/include
LIB_DIR = -L$(HOME)/src/lib
ORALIB=$(ORACLE)/lib32/libclntsh.so
LIBS= -lrtdb_api -lservices -lpthread -lman -ldbclient
CC = gcc -lpthread 

all: rtdb_test

rtdb_test: $(OBJS)
	$(CC) -o ./rtdb_test $(OBJS) $(INC_DIR) $(LIB_DIR)  $(LIBS) 

.cxx.o:
	$(CC) -g -c $(INC_DIR) -o $@ $<

clean:
	rm -f *.o
.PHONY: clean

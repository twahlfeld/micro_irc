CC  = gcc
CXX = clang++

INCLUDES = 
CFLAGS   = -g -Wall $(INCLUDES)
CXXFLAGS = -g -Wall $(INCLUDES)

LDFLAGS = -g
LDLIBS  = -lpthread

.PHONY: default
default: server client
	rm -rf *~ a.out *.o *dSYM

server: server.o auth.o list.o srv_io.o

client: client.o

client.o: client.c

server.o: server.c server.h

auth.o: auth.c auth.h

list.o: list.c list.h

srv_io.o: srv_io.c srv_io.h

.PHONY: clean
clean:
	rm -rf *~ a.out *.o core server client *dSYM

.PHONY: all
all: clean default

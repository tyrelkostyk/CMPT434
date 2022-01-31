.PHONY: all clean clients server

# Verbose, i.e. whether the compiler will define the DEBUG macro
V=0

TCP_SRV=tcp_server

SRCFILES=$(TCP_SRV).c
# HDRFILES = defs.h
OBJFILES=$(TCP_SRV).o

CFLAGS=-Wall -pthread

all: $(TCP_SRV).out

$(TCP_SRV).out: $(TCP_SRV).o
	gcc $(CFLAGS) $(TCP_SRV).o -o $(TCP_SRV).out

$(TCP_SRV).o: $(TCP_SRV).c # $(HDRFILES)
ifeq ($(V), 1)
	gcc $(CFLAGS) -DDEBUG -c $(TCP_SRV).c
else
	gcc $(CFLAGS) -c $(TCP_SRV).c
endif

clean:
	rm -f *.o $(TCP_SRV).out

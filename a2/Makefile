.PHONY: all clean clients server

# Verbose, i.e. whether the compiler will define the DEBUG macro
V=0

UDP_RECV=udp_receiver
UDP_SEND=udp_sender

SRCFILES=$(UDP_RECV).c $(UDP_SEND).c
# HDRFILES = defs.h
OBJFILES=$(UDP_RECV).o $(UDP_SEND).O

CFLAGS=-Wall

all: $(UDP_RECV).out $(UDP_SEND).out

$(UDP_RECV).out: $(UDP_RECV).o
	gcc $(CFLAGS) $(UDP_RECV).o -o $(UDP_RECV).out

$(UDP_RECV).o: $(UDP_RECV).c
ifeq ($(V), 1)
	gcc $(CFLAGS) -DDEBUG -c $(UDP_RECV).c
else
	gcc $(CFLAGS) -c $(UDP_RECV).c
endif

$(UDP_SEND).out: $(UDP_SEND).o
	gcc $(CFLAGS) $(UDP_SEND).o -o $(UDP_SEND).out

$(UDP_SEND).o: $(UDP_SEND).c
ifeq ($(V), 1)
	gcc $(CFLAGS) -DDEBUG -c $(UDP_SEND).c
else
	gcc $(CFLAGS) -c $(UDP_SEND).c
endif

clean:
	rm -f *.o $(UDP_RECV).out $(UDP_SEND).out

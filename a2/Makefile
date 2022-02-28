.PHONY: all clean clients server

# Verbose, i.e. whether the compiler will define the DEBUG macro
V=0

UDP_SRV=udp_server
UDP_CLNT=udp_client

SRCFILES=$(UDP_SRV).c $(UDP_CLNT).c
# HDRFILES = defs.h
OBJFILES=$(UDP_SRV).o $(UDP_CLNT).O

CFLAGS=-Wall

all: $(UDP_SRV).out $(UDP_CLNT).out

$(UDP_SRV).out: $(UDP_SRV).o
	gcc $(CFLAGS) $(UDP_SRV).o -o $(UDP_SRV).out

$(UDP_SRV).o: $(UDP_SRV).c
ifeq ($(V), 1)
	gcc $(CFLAGS) -DDEBUG -c $(UDP_SRV).c
else
	gcc $(CFLAGS) -c $(UDP_SRV).c
endif

$(UDP_CLNT).out: $(UDP_CLNT).o
	gcc $(CFLAGS) $(UDP_CLNT).o -o $(UDP_CLNT).out

$(UDP_CLNT).o: $(UDP_CLNT).c
ifeq ($(V), 1)
	gcc $(CFLAGS) -DDEBUG -c $(UDP_CLNT).c
else
	gcc $(CFLAGS) -c $(UDP_CLNT).c
endif

clean:
	rm -f *.o $(UDP_SRV).out $(UDP_CLNT).out

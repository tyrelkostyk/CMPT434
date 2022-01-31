.PHONY: all clean clients server

# Verbose, i.e. whether the compiler will define the DEBUG macro
V=0

TCP_SRV=tcp_server
TCP_CLNT=tcp_client

SRCFILES=$(TCP_SRV).c $(TCP_CLNT).c
# HDRFILES = defs.h
OBJFILES=$(TCP_SRV).o $(TCP_CLNT).O

CFLAGS=-Wall

all: $(TCP_SRV).out $(TCP_CLNT).out

$(TCP_SRV).out: $(TCP_SRV).o
	gcc $(CFLAGS) $(TCP_SRV).o -o $(TCP_SRV).out

$(TCP_SRV).o: $(TCP_SRV).c # $(HDRFILES)
ifeq ($(V), 1)
	gcc $(CFLAGS) -DDEBUG -c $(TCP_SRV).c
else
	gcc $(CFLAGS) -c $(TCP_SRV).c
endif

$(TCP_CLNT).out: $(TCP_CLNT).o
	gcc $(CFLAGS) $(TCP_CLNT).o -o $(TCP_CLNT).out

$(TCP_CLNT).o: $(TCP_CLNT).c # $(HDRFILES)
ifeq ($(V), 1)
	gcc $(CFLAGS) -DDEBUG -c $(TCP_CLNT).c
else
	gcc $(CFLAGS) -c $(TCP_CLNT).c
endif

clean:
	rm -f *.o $(TCP_SRV).out $(TCP_CLNT).out

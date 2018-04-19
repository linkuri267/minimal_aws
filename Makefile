CPP = g++
CFLAGS = -o

.PHONY: serverA serverB serverC aws monitor

all: 
	$(CPP) $(CFLAGS) aws aws_routines.cpp aws.c
	$(CPP) $(CFLAGS) serverA serverA.cpp
	$(CPP) $(CFLAGS) serverB serverB.cpp
	$(CPP) $(CFLAGS) serverC serverC.cpp
	$(CPP) $(CFLAGS) monitor monitor.c
	$(CPP) $(CFLAGS) client client.c
serverA: 
	./serverA
serverB: 
	./serverB
serverC: 
	./serverC
aws:
	./aws
monitor: 
	./monitor

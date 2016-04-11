stuns = logfile.o typedef.o udp.o Stun.o ProcStat.o Server.o stunMain.o
turns = logfile.o typedef.o udp.o ProcStat.o Server.o Turn.o turnMain.o
messages = logfile.o typedef.o udp.o Message.o ProcStat.o Server.o messageMain.o

all: stun turn message

stun : $(stuns)
	g++ $(stuns) -o stun -lpthread
turn : $(turns)
	g++ $(turns) -o turn -lpthread
message : $(messages)
	g++ $(messages) -o message

logfile.o : logfile.cpp logfile.h
typedef.o : typedef.cpp typedef.h
udp.o : udp.cpp udp.h
Message.o : Message.cpp Message.h
ProcStat.o : ProcStat.cpp ProcStat.h
Server.o : Server.cpp Server.h
Stun.o : Stun.cpp Stun.h
Turn.o : Turn.cpp Turn.h
stunMain.o : stunMain.cpp
turnMain.o : turnMain.cpp
messageMain.o : messageMain.cpp


clean : 
	rm -f $(stuns)
	rm -f $(turns)
	rm -f $(messages)


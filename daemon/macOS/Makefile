SRCS=arduino-serial-lib.c
OBJS=$(subst .c,.o,$(SRCS))

daemon: dependencies
	mkdir -p bin
	g++ -o ./bin/daemon daemon.cpp $(OBJS) -framework ApplicationServices
dependencies:
	gcc -c $(SRCS)
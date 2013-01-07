CC=g++
CFLAGS=-std=c++11 -Wall $$(pkg-config --cflags --libs opencv)
CFLAGSDEBUG=$(CFLAGS) -ggdb
PROGNAME=zhangsuen
PROGDEBUG=zhangsuendebug

all: $(PROGNAME)

debug: $(PROGDEBUG)

clean: 
	rm -f zhangsuen

$(PROGNAME): zhangsuen.cpp zhangsuen.h
	$(CC) $(CFLAGS) zhangsuen.cpp -o $(PROGNAME)

$(PROGDEBUG): zhangsuen.cpp zhangsuen.h
	$(CC) $(CFLAGSDEBUG) zhangsuen.cpp -o $(PROGNAME)




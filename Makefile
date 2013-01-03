CC=g++
CFLAGS=-std=c++11 -Wall $$(pkg-config --cflags --libs opencv)
PROGNAME=zhangsuen

all: $(PROGNAME)

clean: 
	rm -f zhangsuen

$(PROGNAME): zhangsuen.cpp
	$(CC) $(CFLAGS) zhangsuen.cpp -o $(PROGNAME)




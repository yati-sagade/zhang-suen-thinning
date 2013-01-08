CC=g++
CFLAGS=-std=c++11 -Wall -c $$(pkg-config --cflags opencv)
CFLAGSDEBUG=$(CFLAGS) -ggdb -O0
LIBNAME=libzhangsuen.a
LIBDEBUG=libzhangsuendebug.a

all: $(LIBNAME)

debug: $(LIBDEBUG)

clean: 
	rm -f $(LIBNAME)

$(LIBNAME): zhangsuen.cpp zhangsuen.h
	$(CC) $(CFLAGS) zhangsuen.cpp -o $(LIBNAME)

$(LIBDEBUG): zhangsuen.cpp zhangsuen.h
	$(CC) $(CFLAGSDEBUG) zhangsuen.cpp -o $(LIBNAME)




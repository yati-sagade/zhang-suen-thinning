CC=g++
CFLAGS=-std=c++11 -Wall -c $$(pkg-config --cflags opencv)
CFLAGSDEBUG=$(CFLAGS) -ggdb -O0
LIBNAME=libzhangsuen.a
LIBDEBUG=libzhangsuendebug.a
LIBPYTHON=python/zhangsuen.so

PYTHON_VERSION = 2.7
PYTHON_INCLUDE = /usr/include/python$(PYTHON_VERSION)

BOOST_INC = /usr/local/include
BOOST_LIB = /usr/local/lib
OPENCV_LIB = $$(pkg-config --libs opencv)
OPENCV_INC = $$(pkg-config --cflags opencv)

all: $(LIBNAME)

debug: $(LIBDEBUG)

python: $(LIBPYTHON)

clean: 
	rm -f $(LIBNAME)


$(LIBNAME): zhangsuen.cpp zhangsuen.h
	$(CC) $(CFLAGS) zhangsuen.cpp -o $(LIBNAME)

$(LIBDEBUG): zhangsuen.cpp zhangsuen.h
	$(CC) $(CFLAGSDEBUG) zhangsuen.cpp -o $(LIBNAME)


$(LIBPYTHON): python/zhangsuen.o zhangsuen.o
	g++ -shared -Wl,--export-dynamic \
	python/zhangsuen.o zhangsuen.o -L$(BOOST_LIB) -lboost_python -lboost_numpy \
	$(OPENCV_LIB) \
	-L/usr/lib/python$(PYTHON_VERSION)/config -lpython$(PYTHON_VERSION) \
	-o $(LIBPYTHON)


zhangsuen.o: zhangsuen.cpp zhangsuen.h
	g++ -std=c++11 -I$(PYTHON_INCLUDE) $(OPENCV_INC) -I$(BOOST_INC) -I. -fPIC -c zhangsuen.cpp

python/zhangsuen.o: python/wrap_zhangsuen.cpp
	g++ -I$(PYTHON_INCLUDE) $(OPENCV_INC) -I$(BOOST_INC) -I. -fPIC -c python/wrap_zhangsuen.cpp \
	-o python/zhangsuen.o

CC=g++
CFLAGS=-std=c++11 -Wall -c $$(pkg-config --cflags opencv)
CFLAGSDEBUG=$(CFLAGS) -ggdb -O0
LIBNAME=libzhangsuen.a
LIBDEBUG=libzhangsuendebug.a
LIBSHARED=libzhangsuen.o
PY_OUTPUT_DIR=python
PY_OUTPUT_BASE=zhangsuen
LIBPYTHON=$(PY_OUTPUT_DIR)/$(PY_OUTPUT_BASE).so


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
	rm -f $(LIBNAME) $(LIBPYTHON) $(PY_OUTPUT_DIR)/$(PY_OUTPUT_BASE).o $(LIBSHARED)


$(LIBNAME): zhangsuen.cpp zhangsuen.h
	$(CC) $(CFLAGS) zhangsuen.cpp -o $(LIBNAME)

$(LIBDEBUG): zhangsuen.cpp zhangsuen.h
	$(CC) $(CFLAGSDEBUG) zhangsuen.cpp -o $(LIBNAME)


$(LIBPYTHON): $(PY_OUTPUT_DIR)/$(PY_OUTPUT_BASE).o $(LIBSHARED)
	g++ -shared -Wl,--export-dynamic \
	$(PY_OUTPUT_DIR)/$(PY_OUTPUT_BASE).o $(LIBSHARED) -L$(BOOST_LIB) -lboost_python -lboost_numpy \
	$(OPENCV_LIB) \
	-L/usr/lib/python$(PYTHON_VERSION)/config -lpython$(PYTHON_VERSION) \
	-o $(LIBPYTHON)


$(LIBSHARED): zhangsuen.cpp zhangsuen.h
	g++ -std=c++11 -I$(PYTHON_INCLUDE) $(OPENCV_INC) -I$(BOOST_INC) -I. -fPIC -c zhangsuen.cpp -o $(LIBSHARED)

$(PY_OUTPUT_DIR)/$(PY_OUTPUT_BASE).o: python/wrap_zhangsuen.cpp
	g++ -I$(PYTHON_INCLUDE) $(OPENCV_INC) -I$(BOOST_INC) -I. -fPIC -c $(PY_OUTPUT_DIR)/wrap_zhangsuen.cpp \
	-o $(PY_OUTPUT_DIR)/$(PY_OUTPUT_BASE).o

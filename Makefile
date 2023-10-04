CXX=g++
CXXFLAGS = -O2 -Wall -std=c++2a -fPIC

HEADER_DIR = include
CXXFLAGS += -I$(HEADER_DIR)

SRC_DIR=src
SOURCES = file_parsing.cc state_machine.cc actions.cc adapter.cc
SRC := $(addprefix $(SRC_DIR)/, $(SOURCES))

OBJECTS = $(SOURCES:.cc=.o)
OBJ_DIR = build
OBJ := $(addprefix $(OBJ_DIR)/, $(OBJECTS))

DLL = libactions.so

.PHONY: run test

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(DLL): $(OBJ)
	$(CXX) -shared $(CXXFLAGS) $(OBJ) -o $(DLL)

run: $(DLL)
	python3 test.py

$(OBJ_DIR)/file_parsing.o: src/file_parsing.cc include/file_parsing.hh include/utilities.hh

$(OBJ_DIR)/state_machine.o: src/state_machine.cc include/state_machine.hh include/utilities.hh

$(OBJ_DIR)/actions.o: src/actions.cc include/state_machine.hh include/file_parsing.hh include/utilities.hh include/adapter.hh

$(OBJ_DIR)/adapter.o: src/adapter.cc include/adapter.hh include/state_machine.hh include/file_parsing.hh include/utilities.hh

clean_obj:
	rm libactions.so
	rm $(OBJ_DIR)/*.o

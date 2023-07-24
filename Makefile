CXX=g++
CXXFLAGS = -O2 -Wall -std=c++2a -fPIC

HEADER_DIR = include
CXXFLAGS += -I$(HEADER_DIR)

SRC_DIR=src
SOURCES = file_parsing.cc state_machine.cc actions.cc
SRC := $(addprefix $(SRC_DIR)/, $(SOURCES))

OBJECTS = $(SOURCES:.cc=.o)
OBJ_DIR = obj
OBJ := $(addprefix $(OBJ_DIR)/, $(OBJECTS))

DLL = actions.so

.PHONY: 

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(DLL): $(OBJ)
	$(CXX) -shared $(CXXFLAGS) $(OBJ) -o $(DLL)

run: $(DLL)
	python3 get_actions.py

test: $(DLL) src/test.cc
	$(CXX) $(CXXFLAGS) -Wno-sign-compare $(SRC_DIR)/test.cc $(SRC) -o test

$(OBJ_DIR)/file_parsing.o: src/file_parsing.cc include/file_parsing.hh include/utilities.hh

$(OBJ_DIR)/state_machine.o: src/state_machine.cc include/state_machine.hh include/utilities.hh

$(OBJ_DIR)/actions.o: src/actions.cc include/state_machine.hh include/file_parsing.hh include/utilities.hh

clean_obj:
	rm actions.so
	rm $(OBJ_DIR)/*.o

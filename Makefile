CXX = $(shell root-config --cxx)
LD = $(shell root-config --ld)
OS_NAME:=$(shell uname -s | tr A-Z a-z)
ifeq ($(OS_NAME),darwin)
STDINCDIR := -I/opt/local/include
STDLIBDIR := -L/opt/local/lib
else
STDINCDIR := 
STDLIBDIR := 
endif
CPPFLAGS := $(shell root-config --cflags) $(STDINCDIR)
LDFLAGS := $(shell root-config --glibs) $(STDLIBDIR)
CPPFLAGS += -g
TARGET = SiPMLogger
SRC = SiPMLogger.cpp
OBJ = $(SRC:.cpp=.o)
all : $(TARGET)
$(TARGET) : $(OBJ)
	$(LD) $(CPPFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)
%.o : %.cpp
	$(CXX) $(CPPFLAGS) -o $@ -c $<

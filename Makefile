ifneq ($(BUILD),debug)
    BUILD = release
endif

CXX = c++
CC = cc
LD = c++

CXXFLAGS += \
  -Wall \
  -pedantic \
  -std=c++17 \
  -fexceptions \
  -fopenmp \
  -I./src/include \
  -I./src/include/uSockets/src \
  -DLIBUS_USE_LIBUV \
  -DLIBUS_NO_SSL \
  $(shell pkg-config --cflags opencv4)

CFLAGS += \
  -O2 \
  -I./src/include/uSockets/src \
  -DLIBUS_USE_LIBUV \
  -DLIBUS_NO_SSL

LDFLAGS += \
  -lopencv_core \
  -lopencv_video \
  -lopencv_videoio \
  -lgomp \
  -lpthread \
  -lsfml-graphics \
  -lsfml-window \
  -lsfml-system \
  -ltgui \
  -luv \
  -lz \
  -fopenmp

ifeq ($(BUILD),debug)
  OUT_DIR = bin/Debug/
  OBJ_DIR = obj/Debug/src
  CXXFLAGS += -g
  CFLAGS += -g
else
  OUT_DIR = bin/Release/
  OBJ_DIR = obj/Release/src
  CXXFLAGS += -O3
  LDFLAGS += -O3 -s
endif

SOURCE :=  $(wildcard src/*.cpp src/*.h src/userio/*.cpp src/userio/*.h \
  src/userio/sfmlComponents/*.cpp src/userio/sfmlComponents/*.h \
  src/utils/*.cpp src/utils/*.h \
  src/ai/*.cpp src/ai/*.h \
  src/survivalCriteria/*.cpp src/survivalCriteria/*.h \
  src/userio/sfmlComponents/flowComponents/*.cpp src/userio/sfmlComponents/flowComponents/*.h \
  src/userio/sfmlComponents/settingsComponents/*.cpp src/userio/sfmlComponents/settingsComponents/*.h \
  )
CXXSOURCE :=  $(filter %.cpp, $(SOURCE))
HEADERS :=  $(filter %.h, $(SOURCE))
OBJS := $(subst src/,$(OBJ_DIR)/, $(CXXSOURCE:.cpp=.o))
INCLUDES = -I./src/include
LIBS = -L/path/to/cereal/lib -lcereal

# uSockets C sources
USOCKETS_SRC = src/include/uSockets/src
USOCKETS_OBJS = $(OBJ_DIR)/uSockets/context.o $(OBJ_DIR)/uSockets/loop.o \
  $(OBJ_DIR)/uSockets/socket.o $(OBJ_DIR)/uSockets/bsd.o $(OBJ_DIR)/uSockets/udp.o \
  $(OBJ_DIR)/uSockets/eventing/libuv.o


all: debug release


before_debug:
	test -d bin/Debug || mkdir -p bin/Debug
	test -d obj/Debug/src || mkdir -p obj/Debug/src
	test -d obj/Debug/src/userio || mkdir -p obj/Debug/src/userio
	test -d obj/Debug/src/userio/sfmlComponents || mkdir -p obj/Debug/src/userio/sfmlComponents
	test -d obj/Debug/src/userio/sfmlComponents/flowComponents || mkdir -p obj/Debug/src/userio/sfmlComponents/flowComponents
	test -d obj/Debug/src/userio/sfmlComponents/settingsComponents || mkdir -p obj/Debug/src/userio/sfmlComponents/settingsComponents
	test -d obj/Debug/src/utils || mkdir -p obj/Debug/src/utils
	test -d obj/Debug/src/ai || mkdir -p obj/Debug/src/ai
	test -d obj/Debug/src/survivalCriteria || mkdir -p obj/Debug/src/survivalCriteria
	test -d obj/Debug/src/uSockets || mkdir -p obj/Debug/src/uSockets
	test -d obj/Debug/src/uSockets/eventing || mkdir -p obj/Debug/src/uSockets/eventing

before_release:
	test -d bin/Release || mkdir -p bin/Release
	test -d obj/Release/src || mkdir -p obj/Release/src
	test -d obj/Release/src/userio || mkdir -p obj/Release/src/userio
	test -d obj/Release/src/userio/sfmlComponents || mkdir -p obj/Release/src/userio/sfmlComponents
	test -d obj/Release/src/userio/sfmlComponents/flowComponents || mkdir -p obj/Release/src/userio/sfmlComponents/flowComponents
	test -d obj/Release/src/userio/sfmlComponents/settingsComponents || mkdir -p obj/Release/src/userio/sfmlComponents/settingsComponents
	test -d obj/Release/src/utils || mkdir -p obj/Release/src/utils
	test -d obj/Release/src/ai || mkdir -p obj/Release/src/ai
	test -d obj/Release/src/survivalCriteria || mkdir -p obj/Release/src/survivalCriteria
	test -d obj/Release/src/uSockets || mkdir -p obj/Release/src/uSockets
	test -d obj/Release/src/uSockets/eventing || mkdir -p obj/Release/src/uSockets/eventing

.PHONY : release debug
debug: before_debug
	@$(MAKE) --no-print-directory bin/Debug/biosim4 BUILD=$@

release: before_release
	@$(MAKE) --no-print-directory bin/Release/biosim4 BUILD=$@


$(OUT_DIR)biosim4: $(OBJS) $(USOCKETS_OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

$(OBJS): $(HEADERS)

$(OBJ_DIR)%.o : src%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# uSockets compilation rules
$(OBJ_DIR)/uSockets/%.o : $(USOCKETS_SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@


clean: clean_debug clean_release

clean_debug:
	$(RM) -f obj/Debug/src/*
	$(RM) -f obj/Debug/src/uSockets/*
	$(RM) -f obj/Debug/src/uSockets/eventing/*
	$(RM) -f bin/Debug/biosim4

clean_release:
	$(RM) -f obj/Release/src/*
	$(RM) -f obj/Release/src/uSockets/*
	$(RM) -f obj/Release/src/uSockets/eventing/*
	$(RM) -f bin/Release/biosim4

distclean: clean
	$(RM) -f Output/Images/* Output/Videos/* Output/Logs/* Output/Saves/* Output/Profiling/*

.PHONY: all clean distclean

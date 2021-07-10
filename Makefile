SRC := src
INCLUDE := include
BUILD := build
TARGET := cgpr

CC := g++
CXXFLAGS := -I$(SRC) -I$(INCLUDE) -std=c++20 -g -Wall -Wextra -O3
LDFLAGS := -lglfw3

ifeq ($(OS),Windows_NT)
	LDFLAGS += -lopengl32 -mwindows
else
	LDFLAGS += -lGL -Llib -lrt -lm -ldl -pthread -lX11 -lXinerama -lXi -lXxf86vm -lXcursor #Not sure if these are correct, I just copied these from an old Makefile
endif

find = $(shell find $1 -type f -name $2 -print 2> /dev/null)
SRCS := $(call find, $(SRC)/, "*.c") $(call find, $(SRC)/, "*.cpp")
OBJECTS := $(SRCS:%=$(BUILD)/objects/%.o)

vpath %.o $(BUILD)/objects
vpath %.c $(SRC)
vpath %.cpp $(SRC)

$(BUILD)/$(TARGET): $(OBJECTS)
	@echo Linking $@
	@mkdir -p $(BUILD)
	@$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

$(BUILD)/objects/%.c.o: %.c
	@echo Compiling $@
	@mkdir -p $(dir $@)
	@$(CC) -c -o $@ $< $(CXXFLAGS)

$(BUILD)/objects/%.cpp.o: %.cpp
	@echo Compiling $@
	@mkdir -p $(dir $@)
	@$(CC) -c -o $@ $< $(CXXFLAGS)

clean:
	@rm -rf ./$(BUILD)

all: $(BUILD)/$(TARGET)

run: $(BUILD)/$(TARGET)
	@$<

.PHONY: clean all run
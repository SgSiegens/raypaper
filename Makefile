# Makefile for Raypaper

CC ?= gcc

SRC_DIR := src
BUILD_DIR := build
TARGET := raypaper

CFLAGS ?= -std=c11 -D_GNU_SOURCE -O2 -Wall -Wextra -I$(SRC_DIR)

#- mac os needs some different flags for raylib
#- see https://github.com/raysan5/raylib/wiki/Working-on-macOS#:~:text=If-,the,-build%20fails%2C%20you
UNAME_S := $(shell uname -s)

# Only needed if raylib isn't found system-wide, e.g. you built it from
# source instead of installing it: make RAYLIB_DIR=/path/to/raylib/src
RAYLIB_DIR ?=

ifeq ($(UNAME_S), Darwin)
    CFLAGS += -I$(shell brew --prefix raylib)/include
    LIBS := -lraylib -lm -L$(shell brew --prefix raylib)/lib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
else
    LIBS := -lraylib -lm
    # A system-installed (shared) libraylib.so already carries its own
    # X11/GL/pthread/dl/rt dependencies, so -lraylib -lm is enough. A
    # from-source *static* build (RAYLIB_DIR below) does not carry those
    # transitively, so add them explicitly in that case.
    ifneq ($(RAYLIB_DIR),)
        LIBS += -lGL -lpthread -ldl -lrt -lX11
    endif
endif

ifneq ($(RAYLIB_DIR),)
    CFLAGS += -I$(RAYLIB_DIR)
    LDFLAGS += -L$(RAYLIB_DIR)
endif

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS) $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	@echo "Cleaning up..."
	rm -rf $(BUILD_DIR) $(TARGET)

install: $(TARGET)
	mkdir -p $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)

uninstall:
	rm -f $(BINDIR)/$(TARGET)

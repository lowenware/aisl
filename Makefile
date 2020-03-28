#
# Makefile
# Ilja Kartašov, 2019-03-02 17:32
#
.POSIX:

TARGET_NAME = aisl

# Version

VERSION_MAJOR = 1
VERSION_MINOR = 1
VERSION_TWEAK = 0
VERSION_LABEL = 0

# Project directories
SRC_DIR ?= src
OUT_DIR ?= build
DESTDIR ?=
PREFIX ?= /usr/local

PKG_CONFIG ?= pkg-config

# Source files

SOURCE_FILES := \
  $(SRC_DIR)/instance.c \
  $(SRC_DIR)/server.c \
  $(SRC_DIR)/client.c \
  $(SRC_DIR)/stream.c \
  $(SRC_DIR)/http.c \
  $(SRC_DIR)/ssl.c \
  $(SRC_DIR)/list.c \
  $(SRC_DIR)/str-utils.c \
  $(SRC_DIR)/buffer.c \
  $(SRC_DIR)/types.c \


# compilation macro options:

AISL_WITH_DEBUG ?= 0            # disable debug output
AISL_WITH_SSL   ?= 1            # enable SSL support
AISL_WITH_STRINGIFIERS ?= 1     # enable *_to_string functions


# Examples submodule
# include examples.mk

# CFLAGS

CFLAGS := \
  $(CFLAGS) \
  -std=c99 \
  -pedantic \
  -Wall \
  -Wmissing-prototypes \
  -Wstrict-prototypes \
  -Wold-style-definition \
  -O2 \
  -s \
  -fvisibility=hidden \
  -D_POSIX_C_SOURCE=200809L \
  \
  -DVERSION_MAJOR=$(VERSION_MAJOR) \
  -DVERSION_MINOR=$(VERSION_MINOR) \
  -DVERSION_TWEAK=$(VERSION_TWEAK) \
  -DVERSION_LABEL=$(VERSION_LABEL) \
  \
  -DAISL_WITH_DEBUG=$(AISL_WITH_DEBUG) \
  -DAISL_WITH_SSL=$(AISL_WITH_SSL) \
  -DAISL_WITH_STRINGIFIERS=$(AISL_WITH_STRINGIFIERS) \
  \
  -I./ \
  -I./include \

ifeq (${AISL_WITH_SSL}, 1)
  CFLAGS += `$(PKG_CONFIG) --cflags openssl`
  LDFLAGS += `$(PKG_CONFIG) --libs openssl`
endif


# Instructions

SOURCE_LIST := $(wildcard $(SOURCE_FILES))
SHARED_OBJS := $(addsuffix .o, $(addprefix $(OUT_DIR)/shared/, ${SOURCE_LIST}))
STATIC_OBJS := $(addsuffix .o, $(addprefix $(OUT_DIR)/static/, ${SOURCE_LIST}))

all: lib$(TARGET_NAME).so lib$(TARGET_NAME).a
	$(info AISL_WITH_DEBUG=$(AISL_WITH_DEBUG))
	$(info AISL_WITH_SSL=$(AISL_WITH_SSL))
	$(info AISL_WITH_STRINGIFIERS=$(AISL_WITH_STRINGIFIERS))

lib$(TARGET_NAME).so: Makefile $(SHARED_OBJS)
	$(info building target: $@)
	@$(CC) -shared -o $(OUT_DIR)/$@ $(SHARED_OBJS) $(LDFLAGS)
	$(info done: $@)

lib$(TARGET_NAME).a: Makefile $(STATIC_OBJS)
	$(info building target: $@)
	@$(AR) rcs $(OUT_DIR)/$@ $(STATIC_OBJS)
	$(info done: $@)

$(OUT_DIR)/shared/%.o: %
	$(info compiling file: $<)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -fpic -c $< -o $@

$(OUT_DIR)/static/%.o: %
	$(info compiling file: $<)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: install
install:lib$(TARGET_NAME).so lib$(TARGET_NAME).a
	$(info installing files)
	@mkdir -p $(DESTDIR)$(PREFIX)/$(LIB_DIR)
	@mkdir -p $(DESTDIR)$(PREFIX)/include
	@cp $(OUT_DIR)/lib$(LIBRARY_NAME).so $(DESTDIR)$(PREFIX)/$(LIB_DIR)
	@cp $(OUT_DIR)/lib$(LIBRARY_NAME).a $(DESTDIR)$(PREFIX)/$(LIB_DIR)
	@cp -R include/aisl $(DESTDIR)$(PREFIX)/include

.PHONY: clean
clean:
	rm -R ./$(OUT_DIR)/

# -----------------------------------------------------------------------------

QUICKSTART_PATH := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
QUICKSTART_NAME := $(shell basename $$PWD)

quickstart: init run

.PHONY: init
init:
ifneq ($(wildcard Makefile),)
	@echo "ERROR: AISL project could not be initialized in current directory:"
	@echo "Makefile already exists"
	exit 1
endif
	@echo "Initialize $(QUICKSTART_NAME)"
	@cp -r $(QUICKSTART_PATH)bare/Makefile ./
	@cp -r $(QUICKSTART_PATH)bare/src ./

.PHONY: run
run:
	@make -s run

# -----------------------------------------------------------------------------

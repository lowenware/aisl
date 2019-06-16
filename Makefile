#
# Makefile
# Ilja Kartašov, 2019-03-02 17:32
#
.POSIX:

# Project directories
SRC_DIR ?= src
SDK_DIR ?= sdk
OUT_DIR ?= build
LIB_DIR ?= lib
DESTDIR ?=

# Project definition
include project.mk

# Examples submodule
include examples.mk

# CFLAGS

CFLAGS := \
  $(PROJECT_INCLUDES) \
  -std=c99 \
  -pedantic \
  -Wall \
  -Wmissing-prototypes \
  -Wstrict-prototypes \
  -Wold-style-definition \
  -O2 \
  -s \
  -fvisibility=hidden \
  -DVERSION_MAJOR=$(PROJECT_VERSION_MAJOR) \
  -DVERSION_MINOR=$(PROJECT_VERSION_MINOR) \
  -DVERSION_TWEAK=$(PROJECT_VERSION_TWEAK) \
  -DVERSION_LABEL=$(PROJECT_VERSION_LABEL) \
  $(CPPFLAGS) \
  $(CFLAGS) \
  $(PROJECT_CFLAGS) \


LDFLAGS := \
  $(PROJECT_LIBRARIES) \
  $(LDFLAGS) \
  $(PROJECT_LDFLAGS) \


SOURCE_LIST := $(wildcard $(PROJECT_SOURCES))
OBJECT_FILES := $(addprefix $(OUT_DIR)/o_, ${SOURCE_LIST:.c=.o})


library: dirs $(OBJECT_FILES)
	$(info linking target: $@)
	@$(CC) -shared -o $(OUT_DIR)/lib$(PROJECT_NAME).so $(OBJECT_FILES) $(LDFLAGS)
	$(info done: $@)


build/o_%.o: %.c
	$(info compiling file: $<)
	@$(CC) $(CFLAGS) -fpic -c $< -o $@

dirs:
	$(info preparing: build folders)
	@mkdir -p $(OUT_DIR)/o_$(SRC_DIR)
	@mkdir -p $(OUT_DIR)/o_$(SDK_DIR)


clean:
	$(info cleaning: build files)
	@rm -Rf $(OUT_DIR)
	@rm -Rf ./vgcore.*

all: library examples

default: library
.PHONY:  all dirs clean install

install: library
	$(info installing files)
	@mkdir -p $(DESTDIR)$(PREFIX)/$(LIB_DIR)
	@mkdir -p $(DESTDIR)$(PREFIX)/include

	@cp $(OUT_DIR)/lib$(PROJECT_NAME).so $(DESTDIR)$(PREFIX)/$(LIB_DIR)
	@cp -R include/aisl $(DESTDIR)$(PREFIX)/include

# vim:ft=make
#

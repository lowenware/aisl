#
# config.mk
# Löwenware Makefile Config, 2019-03-02 17:35
#

PREFIX ?= /usr/local
PKG_CONFIG ?= pkg-config

PROJECT_NAME = aisl

# Version

PROJECT_VERSION_MAJOR = 1
PROJECT_VERSION_MINOR = 0
PROJECT_VERSION_TWEAK = 0
PROJECT_VERSION_LABEL = 0

#SRC_DIR = src
#SDK_DIR = sdk
#OUT_DIR = ./build


# Source files

PROJECT_SOURCES := \
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


# includes
PROJECT_INCLUDES = -I./ \
       -I./include \
       `$(PKG_CONFIG) --cflags openssl` \

# libraries
PROJECT_LIBRARIES = \
       `$(PKG_CONFIG) --libs openssl` \


# compilation macro options:
# AISL_WITHOUT_SSL - exclude HTTPS support
# AISL_WITHOUT_STRINGIFIERS - exclude several *_to_string functions not

# flags
PROJECT_CFLAGS  = -D_POSIX_C_SOURCE=200809L
#PROJECT_CFLAGS += -DDEBUG
#PROJECT_CFLAGS += -DAISL_WITHOUT_SSL
#PROJECT_CFLAGS += -DAISL_WITHOUT_STRINGIFIERS


# PROJECT_LDFLAGS = -L


# vim:ft=make
#

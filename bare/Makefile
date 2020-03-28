TARGET_PATH := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
TARGET_NAME := $(shell basename $(TARGET_PATH))

SRC_DIR := src/
OUT_DIR := build/

SOURCE_LIST := $(wildcard $(SRC_DIR)*.c)
OBJECT_LIST := $(addsuffix .o, $(addprefix $(OUT_DIR)/, ${SOURCE_LIST}))

TARGET := "$(OUT_DIR)$(TARGET_NAME)"

CFLAGS := \
  $(CFLAGS) \
  -std=c99 \
  -pedantic \
  -Wall \
  -Wmissing-prototypes \
  -Wstrict-prototypes \
  -O2 \
  -s \
  -I./aisl/include \
  -D"TARGET_NAME=\"$(TARGET_NAME)\"" \

all: $(TARGET)

default: $(TARGET)

$(TARGET): Makefile aisl/build/libaisl.a $(OBJECT_LIST)
	$(info building target: $@)
	@$(CC) -o $@ $(OBJECT_LIST) aisl/build/libaisl.a $(LDFLAGS)

$(OUT_DIR)/%.o: %
	$(info compiling file: $<)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

aisl/build/libaisl.a:
	@cd aisl && make -s AISL_WITH_SSL=0 libaisl.a

.PHONY: clean
clean:
	$(info cleaning up: $@)
	@rm -r ./$(OUT_DIR)

.PHONY: run
run: $(TARGET)
	@echo "Run $(TARGET_NAME)"
	./$(TARGET)

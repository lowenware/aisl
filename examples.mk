#
# examples.mk
# Ilja Kartašov, 2019-03-17 17:40
#

EXAMPLES_DIR ?= examples

EXAMPLES_CFLAGS := \
  $(PROJECT_INCLUDES) \
  -std=c99 \
  -pedantic \
  -Wall \
  -Wmissing-prototypes \
  -Wstrict-prototypes \
  -Wold-style-definition \
  -O2 \
  -s \
  $(CFLAGS) \


EXAMPLES_LDFLAGS = -L./ -L./build -laisl -Wl,-rpath=./build

examples: library hello_world

hello_world:
	$(info compiling: hello world)
	@$(CC) $(EXAMPLES_CFLAGS)  \
     -o $(OUT_DIR)/hello-world $(EXAMPLES_DIR)/hello-world.c $(EXAMPLES_LDFLAGS)

# vim:ft=make
#

# Comment/uncomment the following line to disable/enable debugging
DEBUG = y
# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
	DEBFLAGS = -O -g -DDUEL_DEBUG # "-O" is needed to expand inlines
else
	DEBFLAGS = -O2
endif

KERNEL_DIR = /lib/modules/$(shell uname -r)/build

ccflags-y += $(DEBFLAGS)
obj-m := duel.o
duel-objs :=    duel_main.o \
				duel_fast_device.o \
				duel_simple_device.o \
				duel_str_device.o \
				duel_ops_access.o \
				ssd1306/ssd1306_driver.o \
				ssd1306/ssd1306_device.o \
				ssd1306/ssd1306_cmd.o \
				ssd1306/ssd1306_graph.o \
				duel_procfs.o

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

TESTS_DIR = tests
TESTS_SRC = $(wildcard $(TESTS_DIR)/*.c)
TESTS_OUT = $(patsubst $(TESTS_DIR)/%.c,$(TESTS_DIR)/compiled/%.out,$(TESTS_SRC))

tests: $(TESTS_OUT)

$(TESTS_DIR)/compiled/%.out: $(TESTS_DIR)/%.c
	@mkdir -p $(TESTS_DIR)/compiled
	$(CXX) $< -o $@

clean-tests:
	rm -rf $(TESTS_DIR)/compiled

EXAMPLES_DIR = examples
EXAMPLES_SRC = $(wildcard $(EXAMPLES_DIR)/*.c)
EXAMPLES_OUT = $(patsubst $(EXAMPLES_DIR)/%.c,$(EXAMPLES_DIR)/compiled/%.out,$(EXAMPLES_SRC))

examples: $(EXAMPLES_OUT)

$(EXAMPLES_DIR)/compiled/%.out: $(EXAMPLES_DIR)/%.c
	@mkdir -p $(EXAMPLES_DIR)/compiled
	$(CC) $< -o $@

clean-examples:
	rm -rf $(EXAMPLES_DIR)/compiled

.PHONY: tests clean-tests examples clean-examples

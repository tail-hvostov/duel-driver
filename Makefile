# Comment/uncomment the following line to disable/enable debugging
DEBUG = y
# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
	DEBFLAGS = -O -g -DDUEL_DEBUG # "-O" is needed to expand inlines
else
	DEBFLAGS = -O2
endif

KERNEL_DIR = /lib/modules/$(shell uname -r)/build
SRC_DIR = src
BUILD_DIR = build

ccflags-y += $(DEBFLAGS)
obj-m := duel.o
duel-objs :=    $(SRC_DIR)/duel_main.o \
				$(SRC_DIR)/duel_fast_device.o \
				$(SRC_DIR)/duel_simple_device.o \
				$(SRC_DIR)/duel_str_device.o

EXTRA_CFLAGS += $(ccflags-y)

all: $(BUILD_DIR)
	make -C $(KERNEL_DIR) M=$(PWD) modules

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
	rm -rf $(BUILD_DIR)
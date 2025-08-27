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
				ssd1306/ssd1306_cmd.o

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
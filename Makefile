# Comment/uncomment the following line to disable/enable debugging
DEBUG = y
# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
	DEBFLAGS = -O -g -DDUEL_DEBUG # "-O" is needed to expand inlines
else
	DEBFLAGS = -O2
endif

ccflags-y += $(DEBFLAGS)
obj-m := duel.o
duel-objs := duel_main.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

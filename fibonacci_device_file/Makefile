CONFIG_MODULE_SIG = n
TARGET_MODULE := fibonacci

obj-m := $(TARGET_MODULE).o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

ifdef KERNDIR
	BUILDSYSTEM_DIR := $(KERNDIR)
else
	BUILDSYSTEM_DIR := /lib/modules/$(shell uname -r)/build
endif
PWD := $(shell pwd)

all: user
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) clean
	$(RM) user out

load:
	sudo insmod $(TARGET_MODULE).ko

unload:
	sudo rmmod $(TARGET_MODULE).ko || true >/dev/null


user: user.c
	$(CC) -o $@ $^

PRINTF = env printf
PASS_COLOR = \e[32;01m
NO_COLOR = \e[0m
pass = $(PRINTF) "$(PASS_COLOR)$1 Passed [-]$(NO_COLOR)\n"

check: all
	$(MAKE) unload
	$(MAKE) load
	sudo ./user > out
	$(MAKE) unload

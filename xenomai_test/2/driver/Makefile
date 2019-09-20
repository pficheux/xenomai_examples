prefix := $(shell xeno-config --prefix)

ifeq ($(prefix),)
$(error Please add <xenomai-install-path>/bin to your PATH variable)
endif

ifeq ($(KDIR),)
$(error Please define kernel source directory (KDIR))
endif

CC = $(shell xeno-config --cc)
PWD:= $(shell pwd)

EXTRA_CFLAGS += $(shell xeno-config --skin=rtdm --cflags)
EXTRA_CFLAGS += $(CFLAGS)

obj-m += rtdm_test.o

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

install:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules_install
	depmod -a

clean:
	rm -f *~ Module.markers Module.symvers modules.order
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean


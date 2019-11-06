prefix := $(shell xeno-config --prefix)

ifeq ($(prefix),)
$(error Please add <xenomai-install-path>/bin to your PATH variable)
endif

CC := $(shell xeno-config --cc)
STD_CFLAGS  := $(shell xeno-config --alchemy --posix --cflags)
STD_LDFLAGS := $(shell xeno-config --alchemy --posix --ldflags)

STD_TARGETS := alchemy_udd

all: $(STD_TARGETS)

$(STD_TARGETS): $(STD_TARGETS:%=%.c)
	$(CC) -o $@ $< $(STD_CFLAGS) $(STD_LDFLAGS) 

clean:
	$(RM) *.o *~ $(STD_TARGETS)

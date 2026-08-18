.SUFFIXES:

.PHONY: all clean clobber

override CFLAGS += \
	-g \
	-MMD \
	-ffreestanding \
	-Wall \
	-std=c23 \
	-fno-stack-protector \
	-mno-sse \
	--target=x86_64-unknown-none-elf \
	-fpic \
	-fvisibility=hidden

CC := clang

srcfiles := $(patsubst ../Flanterm/src/%,%,$(shell find ../Flanterm/src -type f -name '*.c'))
objs := $(srcfiles:%.c=target/%.o)
depfiles := $(srcfiles:%.c=target/%.d)
outputs := target/flanterm.a

all: $(outputs)

-include $(depfiles)

clean:
	rm -rf target/*

target/flanterm.a: $(objs)
	ar rcs $@ $(objs)

target/%.o: ../Flanterm/src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

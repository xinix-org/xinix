.SUFFIXES:

.PHONY: all clean clobber

override CFLAGS += \
	-g \
	-MMD \
	-ffreestanding \
	-Wall \
	-std=c23 \
	-fno-stack-protector \
	--target=x86_64-unknown-none-elf

override CPPFLAGS += \
	-I include \
	-I include/$(ARCH) \
	-I ../common/include \
	-I ../common/include/$(ARCH) \
	-I $(EXTERNALS)/Flanterm/src

override LDFLAGS += \
	-m elf_x86_64 \
	-nostdlib \
	-static \
	-z max-page-size=0x1000 \
	--gc-sections \
	-T $(LDSCRIPT)

override LIBS = $(PREKERNEL) $(EXTERNALS)/flanterm-build/target/flanterm.a

CC := clang
LD := ld.lld

srcdirs := src/common src/$(ARCH)
srcfiles_c := $(patsubst src/%,%,$(shell find $(srcdirs) -type f -name '*.c'))
srcfiles_asm := $(patsubst src/%,%,$(shell find $(srcdirs) -type f -name '*.S' ))
objs := $(srcfiles_c:%.c=target/%.o) $(srcfiles_asm:%.S=target/%.o)
depfiles := $(srcfiles_c:%.c=target/%.d) $(srcfiles_asm:%.S=target/%.d)
output := target/kernel

all: $(output)

-include $(depfiles)

clean:
	rm -f target/*.o target/*.d

clobber: clean
	rm -f $(output)

target/kernel: $(objs)
	$(LD) $(LDFLAGS) -o $@ $(objs) $(LIBS)

target/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

target/%.o: src/%.S
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

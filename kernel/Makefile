.SUFFIXES:

.PHONY: all clean

LDSCRIPT := link-$(ARCH).ld

override CFLAGS += \
	-g \
	-MMD \
	-ffreestanding \
	-Wall \
	-std=c23 \
	-fno-stack-protector \
	-mno-sse \
	-mno-red-zone \
	--target=x86_64-unknown-none-elf \
	-fpic \
	-fvisibility=protected \

override CPPFLAGS += \
	-I include \
	-I include/$(ARCH) \
	-I ../common/include \
	-I ../common/include/$(ARCH) \
	-I $(EXTERNALS)/Flanterm/src \
	-I ../loader/include 

override LDFLAGS += \
	-m elf_x86_64 \
	-nostdlib \
	-shared \
	-z max-page-size=0x1000 \
	--gc-sections \
	--pack-dyn-relocs=relr \
	-T $(LDSCRIPT) \
	-z now

override LIBS = $(LOADER) $(EXTERNALS)/flanterm-build/target/flanterm.a

CC := clang
LD := ld.lld

srcdirs := src/common src/$(ARCH)
srcfiles_c := $(patsubst src/%,%,$(shell find $(srcdirs) -type f -name '*.c'))
srcfiles_asm := $(patsubst src/%,%,$(shell find $(srcdirs) -type f -name '*.s' ))
objs := $(srcfiles_c:%.c=target/%.o) $(srcfiles_asm:%.s=target/%.o)
depfiles := $(srcfiles_c:%.c=target/%.d) $(srcfiles_asm:%.s=target/%.d)
output := target/xinix-kernel.so

all: $(output)

-include $(depfiles)

clean:
	rm -rf target/*

target/xinix-kernel.so: $(objs)
	$(LD) $(LDFLAGS) -o $@ $(objs) $(LIBS)

target/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

target/%.o: src/%.s
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

.SUFFIXES:

.PHONY: all clean

LDSCRIPT := src/link-$(ARCH).ld

override CFLAGS += \
	-g \
	-MMD \
	-ffreestanding \
	-Wall \
	-std=c23 \
	-fno-stack-protector \
	-mno-sse \
	-mno-red-zone \
	--target=x86_64-unknown-none-elf

override CPPFLAGS += \
	-I include \
	-I ../include \
	-I ../include/$(ARCH) \
	-I ../../common/include \
	-I ../../common/include/$(ARCH) \
	-I ../../loader/include \
	-I ../../kernel/include \
	-I $(EXTERNALS)/limine-protocol/include \
	'-DARCH="$(ARCH)"'

override LDFLAGS += \
	-static \
	-m elf_x86_64 \
	-nostdlib \
	-z max-page-size=0x1000 \
	--gc-sections \
	-T $(LDSCRIPT)

LIBS := $(RANDLIB)

CC := clang
LD := ld.lld
OBJCOPY := llvm-objcopy

objs := target/xinix-kernel-wrapped.o $(patsubst src/%.c,target/%.o,$(shell find src -type f -name '*.c')) $(patsubst ../shared/src/%.c,target/%.o,$(shell find ../shared/src -type f -name '*.c')) $(patsubst ../shared/$(ARCH)/src/%.c,target/%.o,$(shell find ../shared/$(ARCH)/src -type f -name '*.c'))
depfiles := $(objs:target/%.o=target/%.d)
outputs := target/xinix-loader

all: $(outputs)

-include $(depfiles)

clean:
	rm -f target/*

target/xinix-loader: $(objs)
	$(LD) $(LDFLAGS) -o $@ $(objs) $(LIBS)

target/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

target/%.o: ../shared/src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

target/%.o: ../shared/$(ARCH)/src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<


target/xinix-kernel-wrapped.o: $(KERNEL)
	@mkdir -p $(@D)
	@cp -v $< target/ 
	$(OBJCOPY) -I binary -O elf64-x86-64 --set-section-alignment .data=4096 --rename-section .data=.rodata,alloc,load,readonly,data,contents target/$(<F) $@
	@rm -f target/$(<F)

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
	--target=x86_64-unknown-none-elf

override CPPFLAGS += \
	-I include \
	-I ../include \
	-I ../include/$(ARCH) \
	-I ../../common/include \
	-I ../../common/include/$(ARCH) \
	-I ../../kernel/include \
	-I $(EXTERNALS)/limine-protocol/include \
	'-DARCH="$(ARCH)"'

CC := clang

objs := $(patsubst src/%.c,target/%.o,$(shell find src -type f -name '*.c')) $(patsubst ../shared/src/%.c,target/%.o,$(shell find ../shared/src -type f -name '*.c')) $(patsubst ../shared/$(ARCH)/src/%.c,target/%.o,$(shell find ../shared/$(ARCH)/src -type f -name '*.c'))
depfiles := $(objs:target/%.o=target/%.d)
outputs := target/prekernel.a target/link.ld

all: $(outputs)

-include $(depfiles)

clean:
	rm -f target/*.o target/*.d

clobber: clean
	rm -f $(output)

target/prekernel.a: $(objs)
	ar rcs $@ $(objs)

target/%.ld: src/%.ld
	cp $< $@

target/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

target/%.o: ../shared/src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

target/%.o: ../shared/$(ARCH)/src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
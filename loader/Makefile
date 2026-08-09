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
	-I ../common/include \
	-I ../common/include/$(ARCH) \

CC := clang

srcfiles := $(patsubst src/%,%,$(shell find src -type f -name '*.c'))
objs := $(srcfiles:%.c=target/%.o)
depfiles := $(srcfiles:%.c=target/%.d)
outputs := target/loader.a

all: $(outputs)

-include $(depfiles)

clean:
	rm -f target/*.o target/*.d

clobber: clean
	rm -f $(output)

target/loader.a: $(objs)
	ar rcs $@ $(objs)

target/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

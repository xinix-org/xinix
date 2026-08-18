.PHONY: all clean

BOOTLOADER := limine

all:
	@mkdir -p target
	make -C $(BOOTLOADER) ARCH=$(ARCH)
	cp $(BOOTLOADER)/target/xinix-loader target/

clean:
	make -C $(BOOTLOADER) clean
	rm -f target/*

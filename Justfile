build:
    make

clean:
    make clean

run: build
    qemu-system-x86_64 \
        -M q35 \
        -cdrom target/xinix-dev.iso \
        -boot d \
        -m 2G \
        -d int

run-cpu-host: build
    qemu-system-x86_64 \
        -M q35 \
        -cdrom target/xinix-dev.iso \
        -boot d \
        -m 2G \
        -enable-kvm \
        -cpu host

debug: build
    qemu-system-x86_64 \
        -M q35 \
        -cdrom target/xinix-dev.iso \
        -boot d \
        -m 2G \
        -s -S \
        -d int &
    gdb -x debug.gdb

format:
    find . -path './externals' -prune -o \( -name '*.c' -o -name '*.h' \) -print -exec clang-format -i {} \;

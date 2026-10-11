build:
    make

clean:
    make clean

QEMU_ARGS := "-M q35 -cdrom target/xinix-dev.iso -boot d -m 2G"

run: build
    qemu-system-x86_64 \
        {{QEMU_ARGS}}

run-exception-trace: build
    qemu-system-x86_64 \
        {{QEMU_ARGS}} \
        -d int 2> exceptions.err

run-cpu-host: build
    qemu-system-x86_64 \
        {{QEMU_ARGS}} \
        -enable-kvm \
        -cpu host

debug: build
    screen -dmS qemu qemu-system-x86_64 \
        {{QEMU_ARGS}} \
        -s -S
    gdb -x debug-kernel.gdb

debug-prekernel: build
    screen -dmS qemu qemu-system-x86_64 \
        {{QEMU_ARGS}} \
        -s -S
    gdb -x debug-prekernel.gdb

format:
    find . -path './externals' -prune -o \( -name '*.c' -o -name '*.h' \) -print -exec clang-format -i {} \;

format-check:
    find . -path './externals' -prune -o \( -name '*.c' -o -name '*.h' \) -print \( -exec clang-format -Werror --dry-run -i {} \;  -o -quit \)

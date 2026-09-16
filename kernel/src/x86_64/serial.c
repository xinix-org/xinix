#include <limits.h>
#include <stdatomic.h>
#include <stdio.h>
#include <serial.h>

#include <cpu.h>

static size_t serial_read(void* data, size_t len, void* bytes) {
    
}

static size_t serial_write(void* data, size_t len, const void* bytes) {

}

static FILE serial_ports[8] = {
    {
        .data = (void*)0x3F8, 
        .write = serial_write,
        .read = serial_read,
        .close = nullptr,
        .seek = nullptr,
    }
};

static atomic_ulong serial_ports_init;

constexpr unsigned long serial_port_global_init = (unsigned long)LONG_MIN;

int init_serial_ports() {
    return -1;
}

FILE* get_serial(int port) {
    if(port >= 8)
        return nullptr;
    if(!(atomic_load_explicit(&serial_ports_init, memory_order_acquire) & serial_port_global_init)) {
        if(init_serial_ports() < 0)
            return nullptr;
    }
    if(!serial_ports[port].data) {
        // TODO
        return nullptr;
    }

    return &serial_ports[port];
}


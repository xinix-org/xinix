#pragma once

enum irq_number : int {

    IRQ_DEBUG = 0x20,

    IRQ_SPURIOUS = 0x21,

    IRQ_TIMER = 0x28,

    IRQ_KB = 0x41,

};
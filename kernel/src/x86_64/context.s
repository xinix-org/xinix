
.intel_syntax noprefix

.global getcontext, init_context

getcontext:
    mov rax, gs:[0]
    ret

init_context:
    // Must be called exactly once per thread
    mov rdx, rdi
    mov eax, edx
    shr rdx, 32
    mov ecx, 0xC0000101
    wrmsr
    mov ecx, 0xC0000102
    wrmsr
    ret

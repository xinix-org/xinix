
.intel_syntax noprefix

.hidden getcontext, init_context
.global getcontext, init_context

getcontext:
    mov rax, gs:[0]
    ret

init_context:
    mov rdx, rdi
    mov eax, edx
    shr rdx, 32
    mov ecx, 0xC0000101
    wrmsr
    mov ecx, 0xC0000102
    wrmsr
    ret

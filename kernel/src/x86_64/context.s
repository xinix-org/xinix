
.intel_syntax noprefix

.hidden getcontext, init_context, save_full_ucontext, load_full_ucontext, save_debug_ucontext
.global getcontext, init_context, save_full_ucontext, load_full_ucontext, save_debug_ucontext

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


save_full_ucontext:
    mov rax, cr3
    mov rdx, cr4
    mov [rdi+184], rax
    mov [rdi+280], rdx
    str [rdi+156]
    sldt [rdi+158]
    ret

load_full_ucontext:
    lldt [rdi+158]
    ltr [rdi+156]
    mov rdx, [rdi+280]
    mov cr4, rdx
    mov rax, [rdi+184]
    mov cr3, rax
    jmp load_full_ucontext._end
    load_full_ucontext._end:
    ret
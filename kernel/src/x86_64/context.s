
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
    mov rax, dr0
    mov rcx, dr1
    mov rdx, dr2
    mov rsi, dr3
    mov r8, dr6
    mov r9, dr7
    mov [rdi+200], rax
    mov [rdi+208], rcx
    mov [rdi+216], rdx
    mov [rdi+224], rsi
    mov [rdi+232], r8
    mov [rdi+240], r9
    str [rdi+156]
    sldt [rdi+158]
    ret

save_debug_ucontext:
    mov rax, dr0
    mov rcx, dr1
    mov rdx, dr2
    mov rsi, dr3
    mov r8, dr6
    mov r9, dr7
    mov [rdi+200], rax
    mov [rdi+208], rcx
    mov [rdi+216], rdx
    mov [rdi+224], rsi
    mov [rdi+232], r8
    mov [rdi+240], r9
    ret

load_full_ucontext:
    lldt [rdi+158]
    ltr [rdi+156]
    mov rax, [rdi+200]
    mov rcx, [rdi+208]
    mov rdx, [rdi+216]
    mov rsi, [rdi+224]
    mov r8, [rdi+232]
    mov r9, [rdi+240]
    mov dr0, rax
    mov dr1, rcx
    mov dr2, rdx
    mov dr3, rsi
    mov dr6, r8
    mov dr7, r9
    mov rdx, [rdi+280]
    mov cr4, rdx
    mov rax, [rdi+184]
    mov cr3, rax
    jmp load_full_ucontext._end
    load_full_ucontext._end:
    ret
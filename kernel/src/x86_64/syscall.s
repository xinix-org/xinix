
.intel_syntax noprefix

.global SYS_enter_pl0_64, xinix_syslist
.hidden SYS_enter_pl0_64, xinix_syslist

SYS_enter_pl0_64:
    swapgs
    mov r9, gs:[16]
    mov qword ptr [r9+136], r11
    mov r11, r9
    mov qword ptr [r11+128], rcx // return address stored in rcx
    mov qword ptr [r11+24], rbx
    mov qword ptr [r11+32], rsp
    mov qword ptr [r11+40], rbp
    mov qword ptr [r11+96], r12
    mov qword ptr [r11+104], r13
    mov qword ptr [r11+112], r14
    mov qword ptr [r11+120], r15
    mov rsp, qword ptr [r11+288]
    mov rcx, qword ptr gs:[32]
    mov qword ptr gs:[16], rcx
    sti
    mov cx, es
    mov word ptr [r11+144], cx
    mov cx, 0x5B // cs is assumed. `int $0x80` is needed for 32-on-64 syscalls
    mov word ptr [r11+146], cx
    mov cx, ds
    mov word ptr [r11+148], cx
    mov cx, 0x63 // ss is assumed.
    mov word ptr [r11+150], cx
    mov cx, fs
    mov word ptr [r11+152], cx
    mov cx, gs
    mov word ptr [r11+154], cx
    push rax
    push rdx
    mov ecx, 0xC0000100
    rdmsr
    mov [rdi+168], eax
    mov [rdi+172], edx
    mov ecx, 0xC0000102
    rdmsr
    mov [rdi+176], eax
    mov [rdi+180], edx
    mov ecx, 0xC0000101
    rdmsr
    mov ecx, 0x30
    mov fs, cx
    mov gs, cx
    mov ecx, 0xC0000101
    wrmsr
    mov ecx, 0xC0000102
    wrmsr
    pop rdx
    pop rax
    mov rcx, dr0
    mov qword ptr [r11+200], rcx
    mov rcx, dr1
    mov qword ptr [r11+208], rcx
    mov rcx, dr2
    mov qword ptr [r11+216], rcx
    mov rcx, dr3
    mov qword ptr [r11+224], rcx
    mov rcx, dr6
    mov qword ptr [r11+232], rcx
    mov rcx, dr7
    mov qword ptr [r11+240], rcx
    lea rcx, [r11+512]
    mov r9, [rcx-8]
    cmp r9, 512
    jl SYS_enter_pl0_64._no_mxcsr
    stmxcsr dword ptr [rcx+24]
    SYS_enter_pl0_64._no_mxcsr:
    mov r9, r11
    cmp rax, 4096
    jae SYS_enter_pl0_64._nosys
    lea rcx, [xinix_syslist + rip]
    mov rax, qword ptr [rcx + rax * 8]
    test rax, rax
    jz SYS_enter_pl0_64._nosys
    lea rax, [rax + rcx]
    mov rcx, r10
    call rax
    jmp SYS_enter_pl0_64._ret
    SYS_enter_pl0_64._nosys:
    xor edx, edx
    mov rax, -12
    SYS_enter_pl0_64._ret:
    mov r11, qword ptr gs:[24]
    lea rcx, [r11+512]
    mov rsi, [rcx-8]
    cmp rsi, 512
    ja SYS_enter_pl0_64._avx_clear
    jb SYS_enter_pl0_64._no_sse
    xorps xmm0, xmm0
    xorps xmm1, xmm1
    xorps xmm2, xmm2
    xorps xmm3, xmm3
    xorps xmm4, xmm4
    xorps xmm5, xmm5
    xorps xmm6, xmm6
    xorps xmm7, xmm7
    xorps xmm8, xmm8
    xorps xmm9, xmm9
    xorps xmm10, xmm10
    xorps xmm11, xmm11
    xorps xmm12, xmm12
    xorps xmm13, xmm13
    xorps xmm14, xmm14
    xorps xmm15, xmm15
    jmp SYS_enter_pl0_64._end_sse
    SYS_enter_pl0_64._avx_clear:
    vzeroall
    SYS_enter_pl0_64._end_sse:
    ldmxcsr dword ptr [rcx+24]
    SYS_enter_pl0_64._no_sse:
    mov rcx, qword ptr [r11+232]
    mov dr6, rcx
    mov rcx, qword ptr [r11+224]
    mov dr3, rcx
    mov rcx, qword ptr [r11+216]
    mov dr2, rcx
    mov rcx, qword ptr [r11+208]
    mov dr1, rcx
    mov rcx, qword ptr [r11+200]
    mov dr0, rcx
    mov rcx, qword ptr [r11+240]
    mov dr7, rcx
    mov rdi, rax
    mov rsi, rdx
    mov cx, word ptr [r11+144]
    mov es, cx
    mov cx, word ptr [r11+148]
    mov ds, cx
    mov cx, word ptr [r11+152]
    mov fs, cx
    mov bx, word ptr [r11+154]
    mov ecx, 0xC0000101
    rdmsr
    mov gs, bx
    wrmsr
    mov ecx, 0xC0000100
    mov [rdi+168], eax
    mov [rdi+172], edx
    wrmsr
    mov ecx, 0xC0000102
    mov [rdi+176], eax
    mov [rdi+180], edx
    wrmsr
    mov rax, rdi
    mov rdx, rsi
    xor r8, r8
    xor r9, r9
    mov rbx, qword ptr [r11+24]
    mov rbp, qword ptr [r11+40]
    mov r12, qword ptr [r11+96]
    mov r13, qword ptr [r11+104]
    mov r14, qword ptr [r11+112]
    mov r15, qword ptr [r11+120]
    mov r10, qword ptr [r11+136]
    cli
    mov rsp, qword ptr [r11+32]
    mov rcx, qword ptr [r11+128]
    mov r11, r10
    xor r10, r10
    swapgs
    sysretq


.section .text.syscall
// Put system call expressions here

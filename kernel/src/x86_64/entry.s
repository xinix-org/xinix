.intel_syntax noprefix

.global _kstart

.macro relr_apply
    lea r9, [rax+r8]
    mov r12, qword ptr [r9]
    lea r12, [r12+r8]
    mov qword ptr [r9], r12
.endmacro

.type _kstart, @function

_kstart:
    lea rax, [_DYNAMIC+rip]
    mov r8, rax
    mov r10, qword ptr [_GLOBAL_OFFSET_TABLE_+rip]
    sub r8, r10
    xor rbx, rbx
    xor r11, r11
    lea r13, [_kstart._jtbl + rip]
    _kstart._1:
    cmp qword ptr [rax], 0
    je _kstart._relr
    mov r9, qword ptr [rax]
    sub r9, 35
    jb _kstart._2
    cmp r9, 4
    jae _kstart._2
    lea r9, [8*r9 +  r13]
    jmp r9
    _kstart._jtbl:
    // mov rbx, [rax+8]; nop; nop; jmp _kstart._2
    .byte 0x48, 0x8B, 0x58, 0x08, 0x90, 0x90, 0xEB, 0x18
    // mov r11, [rax+8]; nop; nop; jmp _kstart._2
    .byte 0x4C, 0x8B, 0x58, 0x08, 0x90, 0x90, 0xEB, 0x10
    // mov r12, [rax+8]; nop; nop; jmp _kstart._2
    .byte 0x4C, 0x8B, 0x60, 0x08, 0x90, 0x90, 0xEB, 0x08
    // int3; int3; int3; int3; int3; int3; int3; int3
    .byte 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc
    _kstart._2:
    lea rax, [rax+16]
    jmp _kstart._1
    _kstart._relr:
    test rbx, rbx
    je _kstart._relrend
    mov rax, qword ptr [r8+r11]
    lea rbx, [rbx-8]
    lea r11, [r11+8]
    test rax, 1
    jnz _kstart._relr_step
    cmp rax, rax
    relr_apply
    jmp _kstart._relr
    _kstart._relr_step:
    shl rax, 1
    lea r12, [r12+8]
    test rax, rax
    jz _kstart._relr
    test rax, 1
    jz _kstart._relr_step
    relr_apply
    jmp _kstart._relr_step
    _kstart._relrend:
    jmp kmain
    ._kstart._end:

// .size _kstart, _kstart._end-_kstart
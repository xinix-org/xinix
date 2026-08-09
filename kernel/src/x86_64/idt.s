/* Thanks OSDev Wiki! Modified to handle exceptions with error codes, and modified for Intel syntax */

.intel_syntax noprefix
.altmacro

/* Simple sequential list generator, ord determines "order" */
.macro SeqGenerator inst:req, ord:req, n=rsp, va:vararg
  .ifc "\n","rsp"
    .exitm
  .else
    .ifgt \ord /* ord: Non-zero */
      \inst \n
      SeqGenerator \inst \ord \va
    .else /* ord: Zero */
      SeqGenerator \inst \ord \va
      \inst \n
    .endif
  .endif
.endm

.macro IsrStub n:req hascode=0
.global isr_stub\n
.align 16
isr_stub\n:
  swapgs // Always legal
  push rdi // grab a scratch register
  mov rdi, gs:[16] // load the ucontext pointer
  mov [rdi], rax
  mov [rdi+8], rcx
  mov [rdi+16], rdx
  mov [rdi+24], rbx
  mov [rdi+40], rbp
  mov [rdi+48], rsi
  pop qword ptr [rdi+56] //, rdi
  mov [rdi+64], r8
  mov [rdi+72], r9
  mov [rdi+80], r10
  mov [rdi+88], r11
  mov [rdi+96], r12
  mov [rdi+104], r13
  mov [rdi+112], r14
  mov [rdi+120], r15
.ifgt \hascode
  pop rdx // Error code in rdx
.endif
  pop qword ptr [rdi+128] //, rip
  pop rax // cs
  pop qword ptr [rdi+136] //, rflags
  pop qword ptr [rdi+32] //, rsp
  pop rcx // ss
  mov dx, ds
  mov bx, es
  mov [rdi+144], bx
  mov [rdi+146], ax
  mov [rdi+148], dx
  mov [rdi+150], cx
  lea rcx, [rdi+512]
  cmp qword ptr [rcx-8], 512
  jb 1f
  ja 2f
  fxsave64 [rcx]
  jmp 1f
  2:
  mov edx, [rcx-12]
  mov eax, [rcx-16]
  xsave64 [rcx]
  1:
  cld
  mov esi, \n
  mov rbp, rsp
  and rsp, ~15
  .ifgt \hascode /* hascode: Non-zero */
    call handle_int_with_code
  .else /* hascode: Zero */
    call handle_int
  .endif
  mov rsp, rbp
  
  mov rdi, rax
  lea rcx, [rdi+512]
  cmp qword ptr [rcx-8], 512
  jb 1f
  ja 2f
  fxrstor64 [rcx]
  jmp 1f
  2:
  mov edx, [rcx-12]
  mov eax, [rcx-16]
  xrstor64 [rcx]
  1:
  xor eax, eax
  xor ecx, ecx
  mov bx, [rdi+144]
  mov ax, [rdi+146]
  mov dx, [rdi+148]
  mov cx, [rdi+150]
  mov ds, dx
  mov es, bx
  push rcx // , ss
  push qword ptr [rdi+32] //, rsp
  push qword ptr [rdi+136] //, rflags
  push rax // ,cs
  push qword ptr [rdi+128] //, rip
  mov rax, [rdi]
  mov rcx, [rdi+8]
  mov rdx, [rdi+16]
  mov rbx, [rdi+24]
  mov rbp, [rdi+40]
  mov rsi, [rdi+48]
  push qword ptr [rdi+56] //, rdi
  mov r8, [rdi+64]
  mov r9, [rdi+72]
  mov r10, [rdi+80]
  mov r11, [rdi+88]
  mov r12, [rdi+96]
  mov r13, [rdi+104]
  mov r14, [rdi+112]
  mov r15, [rdi+120]
  mov gs:[16], rdi
  pop rdi
  swapgs
  iretq
.endm

.macro IDTEntry n:req
  .word 0 /* offset_low, filled at runtime */
  .word 0b00101000 /* Limine puts a 64-bit identity-mapped code segment in GDT index 5 */
  .byte 0 /* IST */
  .byte 0x8E /* gate type; always using trap gate for now */
  .word 0 /* offset_mid, filled at runtime */
  .long 0 /* offset_high, filled at runtime */
  .long 0 /* reserved */
.endm

.macro IsrListEntry n:req
  .quad isr_stub\n
.endm

.section .text

.set i,0
.rept 8
  IsrStub %i
  .set i,i+1
.endr
IsrStub 8 1
IsrStub 9
.set i,10
.rept 5
  IsrStub %i 1
  .set i,i+1
.endr
IsrStub 15
IsrStub 16
IsrStub 17 1
IsrStub 18
IsrStub 19
IsrStub 20
IsrStub 21 1
.set i,22
.rept 234
  IsrStub %i
  .set i,i+1
.endr

.section .data

.global IDT
IDT:
.align 16
.set i,0
.rept 256
  IDTEntry %i
  .set i,i+1
.endr

.global isr_list
isr_list:
.align 8
.set i,0
.rept 256
  IsrListEntry %i
  .set i,i+1
.endr

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
  /* Swapgs if needed (checks current ring) */
  /* Commented out for now because we're in Ring 0 right now, and I'm not sure how this interacts with error codes. */
  /* TODO */
  /*
  cmpb $0x08, 0x8(%rsp)
  je 1f
  swapgs
1:
  */
  /* Saving is manually required in x86_64, no pushaq exists */
  SeqGenerator push 1 r15 r14 r13 r12 r11 r10 r9 r8 rbp rdi rsi rdx rcx rbx rax
  /* TODO: fxsave */

  mov rdi, rsp /* Argument 1: Interrupt frame */
  mov rsi, \n /* Argument 2: IRQ# */

  .ifgt \hascode /* hascode: Non-zero */
    call handle_int_with_code
  .else /* hascode: Zero */
    call handle_int
  .endif

  /* TODO: fxrstor */
  SeqGenerator pop 0 r15 r14 r13 r12 r11 r10 r9 r8 rbp rdi rsi rdx rcx rbx rax
  /* Swapgs if needed */
  /* Commented out; see above */
  /* TODO */
  /*
  cmpb $0x08, 0x8(%rsp)
  je 1f
  swapgs
1:
  */
  .ifgt \hascode
    add rsp, 8
  .endif
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

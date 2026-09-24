.intel_syntax noprefix
.global _test_prg_start, _test_prg_end


_test_prg_start:
.quad _test_prg_text_end-_test_prg_text
.quad _test_prg_data_end-_test_prg_data
.quad 0
.quad _test_prg_entry-_test_prg_start
_test_prg_text:
_test_prg_entry:
    lea rdi, [_test_prg_hello+rip]
    mov rsi, 12
    mov rax, 1
    syscall
    mov rax, 0
    mov rdi, 0
    syscall
    ud2
_test_prg_hello:
.ascii "Hello World!"

_test_prg_text_end:
_test_prg_data:
_test_prg_data_end:
_test_prg_end:


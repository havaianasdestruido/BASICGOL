    .text
    .globl bg_asm_soma
    .type bg_asm_soma, @function
bg_asm_soma:
    leal (%rdi,%rsi), %eax
    ret
    .size bg_asm_soma, .-bg_asm_soma
    .section .note.GNU-stack,"",@progbits

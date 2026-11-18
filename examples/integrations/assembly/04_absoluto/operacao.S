    .text
    .globl bg_asm_absoluto
    .type bg_asm_absoluto, @function
bg_asm_absoluto:
    movl %edi, %eax
    testl %eax, %eax
    jns .Lbg_abs_pronto
    negl %eax
.Lbg_abs_pronto:
    ret
    .size bg_asm_absoluto, .-bg_asm_absoluto
    .section .note.GNU-stack,"",@progbits

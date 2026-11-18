    .text
    .globl bg_asm_maior
    .type bg_asm_maior, @function
bg_asm_maior:
    cmpl %esi, %edi
    cmovl %esi, %edi
    movl %edi, %eax
    ret
    .size bg_asm_maior, .-bg_asm_maior
    .section .note.GNU-stack,"",@progbits

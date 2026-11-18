    .text
    .globl bg_asm_multiplica
    .type bg_asm_multiplica, @function
bg_asm_multiplica:
    imull %esi, %edi
    movl %edi, %eax
    ret
    .size bg_asm_multiplica, .-bg_asm_multiplica
    .section .note.GNU-stack,"",@progbits

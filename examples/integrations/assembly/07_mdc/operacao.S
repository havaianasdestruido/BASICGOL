    .text
    .globl bg_asm_mdc
    .type bg_asm_mdc, @function
bg_asm_mdc:
.Lbg_mdc_loop:
    testl %esi, %esi
    je .Lbg_mdc_fim
    movl %esi, %ecx
    movl %edi, %eax
    cltd
    idivl %esi
    movl %ecx, %edi
    movl %edx, %esi
    jmp .Lbg_mdc_loop
.Lbg_mdc_fim:
    movl %edi, %eax
    ret
    .size bg_asm_mdc, .-bg_asm_mdc
    .section .note.GNU-stack,"",@progbits

    .text
    .globl bg_asm_soma_vetor
    .type bg_asm_soma_vetor, @function
bg_asm_soma_vetor:
    xorl %eax, %eax
    xorl %ecx, %ecx
.Lbg_soma_vetor_loop:
    cmpq %rsi, %rcx
    jae .Lbg_soma_vetor_fim
    addl (%rdi,%rcx,4), %eax
    incq %rcx
    jmp .Lbg_soma_vetor_loop
.Lbg_soma_vetor_fim:
    ret
    .size bg_asm_soma_vetor, .-bg_asm_soma_vetor
    .section .note.GNU-stack,"",@progbits

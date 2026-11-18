    .section .rodata
    .p2align 3
.Lbg_meio:
    .double 0.5
    .text
    .globl bg_asm_media
    .type bg_asm_media, @function
bg_asm_media:
    addsd %xmm1, %xmm0
    mulsd .Lbg_meio(%rip), %xmm0
    ret
    .size bg_asm_media, .-bg_asm_media
    .section .note.GNU-stack,"",@progbits

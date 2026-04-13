; in samples/hello-pie.asm

        ; 👇 that's new
        default rel

        global _start

        section .text

_start: mov rdi, 1      ; stdout fd
        lea rsi, [rel msg]
        ; mov rsi, msg  ; this will not work without relocation in loading... (loader but do work to patch this...)
        mov rdx, 9      ; 8 chars + newline
        mov rax, 1      ; write syscall
        syscall

        xor rdi, rdi    ; return code 0
        mov rax, 60     ; exit syscall
        syscall

        section .data

msg:    db "hi there", 10
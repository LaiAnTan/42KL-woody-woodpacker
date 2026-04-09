global _start

section .text

_start:
        mov rax, 0x0a68757262 ; "bruh\n" (little endian)
        push rax

        mov rdi, 1          ; stdout
        mov rsi, rsp        ; pointer to string
        mov rdx, 5          ; length
        mov rax, 1          ; write
        syscall

        add rsp, 8          ; clean stack

        xor rdi, rdi        ; exit code 0
        mov rax, 60         ; exit
        syscall
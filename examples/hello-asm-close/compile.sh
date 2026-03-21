nasm -f elf64 hello.asm
# can change section start of .text2
# ld --section-start .data=0x401030 --section-start .text=0x401000 --section-start .text2=0x402000  hello.o -o hello
ld -T link.ld hello.o -o hello


# TODO: do this!!!
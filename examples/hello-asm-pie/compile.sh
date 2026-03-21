nasm -f elf64 hello-pie.asm
ld --dynamic-linker=/lib64/ld-linux-x86-64.so.2 -pie hello-pie.o -o hello-pie
#yasm -f elf64 -g dwarf2 -l   exit.lst exit.asm
#ld -o exit exit.o
#gcc -no-pie  -o   exit exit.o
#./exit
#nasm -f elf exit.asm
#ld exit.o -o test
#gcc exit.o -o test
#./test
gcc  spec.c
./a.out
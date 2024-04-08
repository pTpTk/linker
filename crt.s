 .globl _start
_start:
 movl  $0, %ebp
 call  main
 
 movl  %eax, %ebx
 movl  $1, %eax
 int   $0x80
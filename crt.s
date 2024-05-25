.globl _start
_start:
 xor   %rbp, %rbp
 call  main
 
 movl  %eax, %edi
 movl  $60, %eax
 syscall
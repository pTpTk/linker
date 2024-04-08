 .globl main
main:
 push   %ebp
 movl   %esp, %ebp
 movl   $2, %eax
 movl   %ebp, %esp
 pop    %ebp
 ret
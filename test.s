.globl main
main:
 push   %rbp
 movq   %rsp, %rbp
 movl   $2, %eax
 movq   %rbp, %rsp
 pop    %rbp
 ret
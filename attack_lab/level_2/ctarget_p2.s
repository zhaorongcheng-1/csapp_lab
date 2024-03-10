mov $0x59b997fa, %rdi
sub $0x8, %rsp
mov $0x014017ec, %rax
shl $0x8, %eax
shr $0x8, %eax
mov %rax, (%rsp)
ret

mov $0x5561dc50, %rdi
sub $0x8, %rsp
movq $0x4018fa, (%rsp)
movq $0x39623935, (%rdi)
movq $0x61663739, 4(%rdi)
movb $0x0, 8(%rdi)
ret

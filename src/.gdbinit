target remote localhost:1234
add-symbol-file kernel.elf 0x10000
add-symbol-file test.elf 0x80000000

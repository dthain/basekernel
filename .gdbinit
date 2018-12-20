target remote localhost:1234
add-symbol-file kernel/kernel.elf 0x10000
add-symbol-file user/shell.exe 0x80000000

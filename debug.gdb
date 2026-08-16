target remote localhost:1234
add-symbol-file prekernel/target/xinix-loader
set disassemble-next-line on
set disassembly-flavor intel
break pkmain
continue

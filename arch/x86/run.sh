mkdir -p build
nasm -f bin ./boot/boot.asm -o ./build/TermOS.bin &&
qemu-system-x86_64 -usb ./build/TermOS.bin

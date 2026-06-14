IMAGE = boot.bin
ASM_OBJ = target/primary.o
RUST_BOOT_LIB = target/i686-termos/release/libsecondary_boot.a

.PHONY: all clean run

all: $(IMAGE)

# 1. Собираем 16-битный первый сектор
$(ASM_OBJ): boot/primary.asm
	@mkdir -p target
	nasm -f elf32 boot/primary.asm -o $(ASM_OBJ)

# 2. Передаем Cargo флаг для работы с кастомным JSON и включаем автоматический сбор libcore
cargo_workspace:
	cargo build --release --workspace

# 3. Линкуем первый сектор и вторичный загрузчик на Rust в финальный boot.bin
$(IMAGE): $(ASM_OBJ) cargo_workspace
	ld -m elf_i386 -T linker.ld --oformat binary $(ASM_OBJ) $(RUST_BOOT_LIB) -o $(IMAGE)

clean:
	cargo clean
	rm -rf target $(IMAGE)

run: all
	qemu-system-i386 -drive format=raw,file=$(IMAGE)

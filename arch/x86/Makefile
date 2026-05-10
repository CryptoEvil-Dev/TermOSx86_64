AS = nasm
CC = g++
LD = ld

ASFLAGS = -f elf64
CFLAGS = -m64 -ffreestanding -fno-stack-protector -mno-red-zone -fno-exceptions -fno-rtti -mcmodel=large -c -fno-pic

# Поиск исходников
CPP_SOURCES = $(shell find kernel -name '*.cpp')
ASM_SOURCES = $(shell find kernel -name '*.asm')

# Преобразование путей в объекты (внутри папки build/)
CPP_OBJECTS = $(patsubst kernel/%.cpp, build/%.o, $(CPP_SOURCES))
ASM_OBJECTS = $(patsubst kernel/%.asm, build/%.o, $(ASM_SOURCES))
BOOT_OBJ = build/boot.o

BIN = build/TermOS.bin


DISK_IMG = build/disk.img
DISK_SIZE = 256M

.PHONY: all run clean prepare_disk

all: run

# Команда для подготовки диска
prepare_disk:
	@mkdir -p build
	@if [ ! -f $(DISK_IMG) ]; then \
		echo "[DISK] Creating 256MB disk..."; \
		qemu-img create -f raw $(DISK_IMG) $(DISK_SIZE); \
		mkfs.vfat -F 32 -I $(DISK_IMG); \
		echo "TermOS Test File" > build/hello.txt; \
		mcopy -i $(DISK_IMG) build/hello.txt ::/HELLO.TXT; \
		mcopy -i $(DISK_IMG) build/hello.txt ::/KERNEL.BIN; \
	fi


# Ссылка всех объектов
$(BIN): $(BOOT_OBJ) $(CPP_OBJECTS) $(ASM_OBJECTS)
	@echo "[LINK] $@"
	$(LD) -T linker.ld -o $(BIN) $(BOOT_OBJ) $(CPP_OBJECTS) $(ASM_OBJECTS) -Map=build/output.map

# 1. Правило для C++ файлов
build/%.o: kernel/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Ikernel/include $< -o $@

# 2. Правило для ASM файлов ядра (interrupts.asm и др.)
build/%.o: kernel/%.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# 3. Отдельное правило для загрузчика
$(BOOT_OBJ): boot/boot.asm
	@mkdir -p build
	$(AS) $(ASFLAGS) boot/boot.asm -o $(BOOT_OBJ)


run: $(BIN) prepare_disk
	@echo "[RUN] Launching QEMU..."
	qemu-system-x86_64 -drive format=raw,file=$(BIN) \
	                   -drive format=raw,file=$(DISK_IMG),if=ide,index=1,media=disk


clean:
	rm -rf build/*
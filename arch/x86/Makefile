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

FINAL_IMG=build/flash.img

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

# 3. ГЛАВНОЕ ПРАВИЛО: Создание полноценного образа жесткого диска/флешки
image: $(BIN) prepare_disk
	@echo "[IMAGE] Creating unified flash image: $(FINAL_IMG)..."
	@# Создаем пустой файл образа, равный: размер ядра/загрузчика (в пределах 64 секторов) + размер FAT32
	@# Сектор 64 — это ровно 32 КБ от начала диска
	dd if=/dev/zero of=$(FINAL_IMG) bs=512 count=64 status=none
	@# Записываем загрузчик и ядро в самое начало (с 0-го сектора)
	dd if=$(BIN) of=$(FINAL_IMG) bs=512 conv=notrunc status=none
	@# Склеиваем с томом FAT32, начиная строго с 64-го сектора
	dd if=$(DISK_IMG) of=$(FINAL_IMG) bs=512 seek=64 conv=notrunc status=none
	@# Создаем правильную MBR разметку: помечаем раздел с 64-го сектора как активный FAT32 (тип 0x0c)
	@echo -e "n\np\n1\n64\n\nt\nc\na\n1\nw" | fdisk $(FINAL_IMG) > /dev/null 2>&1
	@echo "[IMAGE] Done! $(FINAL_IMG) is ready for burning or emulating."

# run: image
# 	@echo "[RUN] Launching QEMU with unified $(FINAL_IMG)..."
# 	qemu-system-x86_64 -drive format=raw,file=$(FINAL_IMG),if=ide,index=0,media=disk

clean:
	rm -rf build/*
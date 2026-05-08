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

all: $(BIN) run

# Ссылка всех объектов
$(BIN): $(BOOT_OBJ) $(CPP_OBJECTS) $(ASM_OBJECTS)
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

run:
	qemu-img create -f raw build/disk.img 256M
	qemu-system-x86_64 -drive format=raw,file=$(BIN) -drive file=build/disk.img,format=raw,if=ide

clean:
	rm -rf build/*

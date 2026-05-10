#include "../include/ata.hpp"
#include "../include/io.hpp"
#include "../include/iostream.hpp"

namespace Storage {
    DiskManager disk_mgr;

    void DiskManager::init() {
        std::cout << "Scanning ATA buses..." << std::endl;
        // Сканируем Primary канал
        register_driver(0x1F0, ATA_MASTER);
        register_driver(0x1F0, ATA_SLAVE);
        // Сканируем Secondary канал
        register_driver(0x170, ATA_MASTER);
        register_driver(0x170, ATA_SLAVE);
    }

    void DiskManager::register_driver(uint16_t port, DriveType type) {
        // Команда IDENTIFY
        outb(port + 6, (type == ATA_MASTER) ? 0xA0 : 0xB0);
        outb(port + 2, 0);
        outb(port + 3, 0);
        outb(port + 4, 0);
        outb(port + 5, 0);
        outb(port + 7, 0xEC); // IDENTIFY command

        uint8_t status = inb(port + 7);
        if(status == 0) return; // Диска нет

        // Ждём готовности
        while (inb(port + 7) & 0x80);

        if(inb(port + 4) != 0 || inb(port + 5) != 0) return; // Не ATA диск

        // Читаем 256 слов данных (паспорт диска)
        uint16_t data[256];
        for(int i = 0; i < 256; i++) {
            data[i] = inw(port);
        }

        drives[drive_count].base_port = port;
        drives[drive_count].type = type;
        drives[drive_count].exists = true;
        drives[drive_count].size_sectors = *((uint32_t*)&data[60]); // LBA28 размер

        std::cout << "Found Disk: " << (type == ATA_MASTER ? "Master" : "Slave")
            << " at " << std::hex << port << " Size "
            << std::dec << (drives[drive_count].size_sectors * 512 / 1024 / 1024) << " MB" << std::endl;
        
        drive_count++;
    }

    bool DiskManager::read_sector(uint8_t drive_idx, uint32_t lba, uint8_t* buffer) {
        if (drive_idx >= drive_count) return false;
        Drive& d = drives[drive_idx];

        outb(d.base_port + 6, 0xE0 | ((d.type == ATA_SLAVE) << 4) | ((lba >> 24) & 0x0F));
        outb(d.base_port + 2, 1); // Читаем 1 сектор
        outb(d.base_port + 3, (uint8_t)lba);
        outb(d.base_port + 4, (uint8_t)(lba >> 8));
        outb(d.base_port + 5, (uint8_t)(lba >> 16));
        outb(d.base_port + 7, 0x20); // Команда "Read Sectors with retry"

        ata_wait_ready(d.base_port);

        // Читаем данные (256 слов = 512 байт)
        uint16_t* ptr = (uint16_t*)buffer;
        for (int i = 0; i < 256; i++) {
            ptr[i] = inw(d.base_port);
        }

        return true;
    }


    static void ata_wait_ready(uint16_t port) {
        while (inb(port + 7) & 0x80); // Wait for BSY to clear
        while (!(inb(port + 7) & 0x40)); // Wait for DRDY to set
    }

    bool DiskManager::write_sector(uint8_t drive_idx, uint32_t lba, uint8_t* buffer) {
        if (drive_idx >= drive_count) return false;
        Drive& d = drives[drive_idx];
        
        outb(d.base_port + 6, 0xE0 | ((d.type == ATA_SLAVE) << 4) | ((lba >> 24) & 0x0F));
        outb(d.base_port + 2, 1);
        outb(d.base_port + 3, (uint8_t)lba);
        outb(d.base_port + 4, (uint8_t)(lba >> 8));
        outb(d.base_port + 5, (uint8_t)(lba >> 16));
        outb(d.base_port + 7, 0x30); // Команда "Write Sectors"
        
        ata_wait_ready(d.base_port);
        
        // Передаем данные диску
        uint16_t* ptr = (uint16_t*)buffer;
        for (int i = 0; i < 256; i++) {
            outw(d.base_port, ptr[i]);
        }
    
        // Ждем, пока диск закончит запись на физический блин
        outb(d.base_port + 7, 0xE7); // Каш-флаш (на всякий случай)
        ata_wait_ready(d.base_port);
    
        return true;
    }

}
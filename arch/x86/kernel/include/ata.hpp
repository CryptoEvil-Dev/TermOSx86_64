#pragma once
#include <stdint.h>

namespace Storage {

enum DriveType {
    ATA_MASTER = 0,
    ATA_SLAVE = 1,
};

struct Drive {
    uint16_t base_port;
    DriveType type;
    bool exists;
    char model[41];
    uint64_t size_sectors;
};

class DiskManager {
public:
    void init();
    void register_driver(uint16_t port, DriveType type);

    // Чтение/Запись секторов
    bool read_sector(uint8_t drive_idx, uint32_t lba, uint8_t* buffer);
    bool write_sector(uint8_t drive_idx, uint32_t lba, uint8_t* buffer);

private:
    Drive drives[4]; // Поддержим до 4-х дисков
    uint8_t drive_count = 0;
};

static void ata_wait_ready(uint16_t port);

extern DiskManager disk_mgr;
}
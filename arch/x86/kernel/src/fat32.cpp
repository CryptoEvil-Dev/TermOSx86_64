#include "../include/fat32.hpp"
#include "../include/ata.hpp"
#include "../include/iostream.hpp"
#include "../include/string.hpp"

namespace Storage {

    void fat32_init(uint8_t drive_idx) {
        uint8_t buffer[512];
    
    if (!disk_mgr.read_sector(drive_idx, 0, buffer)) {
        std::cerr << "[FAT32] Read Error!" << std::endl;
        return;
    }

    // Сигнатура 0xAA55 находится в самом конце 512-байтового сектора
    if (buffer[510] != 0x55 || buffer[511] != 0xAA) {
        std::cerr << "[FAT32] Invalid boot signature: " 
                  << std::hex << (int)buffer[510] << " " << (int)buffer[511] << std::endl;
        return;
    }

    fat32_bpb_t* bpb = (fat32_bpb_t*)buffer;


    // Сектор, где начинается первая копия FAT
    fat_start_sector = bpb->reserved_sectors;
    
    // Сектор, где начинаются данные (после всех FAT)
    data_start_sector = bpb->reserved_sectors + (bpb->fat_count * bpb->fat_size_32);
    
    // Начальный сектор корневой директории
    // Формула: DataStart + (RootCluster - 2) * SectorsPerCluster
    root_dir_sector = data_start_sector + (bpb->root_cluster - 2) * bpb->sectors_per_cluster;

    std::cout << "[FAT32] Root Sector: " << root_dir_sector << std::endl;



    std::cout << "[FAT32] Found Drive " << (int)drive_idx << std::endl;

        std::cout << "[FAT32] Initializing Drive " << (int)drive_idx << std::endl;
        std::cout << "        OEM Name: " << bpb->oem_name << std::endl;
        std::cout << "        Bytes per sector: " << bpb->bytes_per_sector << std::endl;
        std::cout << "        Sectors per cluster: " << (int)bpb->sectors_per_cluster << std::endl;
        std::cout << "        FAT Size: " << bpb->fat_size_32 << " sectors" << std::endl;
        std::cout << "        Root Cluster: " << bpb->root_cluster << std::endl;
    }

    void fat32_ls() {
        uint8_t buffer[512];
        // Читаем самый первый сектор корневой директории
        // disk_mgr.read_sector(1, get_sector_by_cluster(current_dir_cluster), buffer);
        uint32_t sector = get_sector_by_cluster(current_dir_cluster);
        disk_mgr.read_sector(1, sector, buffer);


        fat32_dir_entry_t* entry = (fat32_dir_entry_t*)buffer;

        std::cout << "Directory listing for cluster " << current_dir_cluster << ":" << std::endl;
        for (int i = 0; i < 16; i++) { // В секторе помещается 16 записей (512 / 32)
            if (entry[i].name[0] == 0x00) break; // Конец списка
            if (entry[i].name[0] == 0xE5) continue; // Удаленный файл

            // Выводим имя (оно в FAT всегда 11 символов, дополненных пробелами)
            for(int j=0; j<11; j++) std::cout << entry[i].name[j];

            if (entry[i].attributes & 0x10) std::cout << " <DIR>";
            else std::cout << " " << std::dec << entry[i].file_size << " bytes";

            std::cout << std::endl;
        }
    }

    bool fat32_find_file(const char* name, fat32_dir_entry_t* out_entry) {
        uint8_t buffer[512];
        uint32_t sector = get_sector_by_cluster(current_dir_cluster);
        
        if (!disk_mgr.read_sector(1, sector, buffer)) return false;
        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)buffer;

        for (int i = 0; i < 16; i++) {
            uint8_t first_byte = (uint8_t)entries[i].name[0];

            // Дебаг: показываем, что именно мы собираемся сравнивать
            if (first_byte != 0 && first_byte != 0xE5) {
                std::cout << "Slot " << i << " is valid. Comparing..." << std::endl;

                if (fat_name_match(name, entries[i].name)) {
                    *out_entry = entries[i];
                    return true;
                }
            }
        }
        return false;
    }


    bool fat_name_match(const char* input, const char* fat_name) {
        // Выведем каждый байт того, что пришло с диска
        std::cout << "DEBUG: [";
        for(int i=0; i<8; i++) {
            if(fat_name[i] == ' ') std::cout << '_'; // Пробелы заменим на подчеркивание
            else std::cout << (char)fat_name[i];
        }
        std::cout << "]" << std::endl;

        // ВРЕМЕННО: пусть возвращает true для всего, что начинается на 'T'
        if (input[0] == 'T' || input[0] == 't') {
            if (fat_name[0] == 'T') return true;
        }

        return false; 
    }




    void fat32_cat(const char* filename) {
        uint8_t buffer[512];
        disk_mgr.read_sector(1, root_dir_sector, buffer);
        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)buffer;

        for (int i = 0; i < 16; i++) {
            if (fat_name_match(filename, entries[i].name)) {
                uint32_t cluster = ((uint32_t)entries[i].cluster_high << 16) | entries[i].cluster_low;
                uint32_t sector = data_start_sector + (cluster - 2); // Считаем, что кластер = 1 сектор

                uint8_t data[512];
                disk_mgr.read_sector(1, sector, data);

                // Выводим ровно столько, сколько весит файл
                for (uint32_t j = 0; j < entries[i].file_size; j++) {
                    std::cout << (char)data[j];
                }
                std::cout << std::endl;
                return;
            }
        }
        std::cerr << "File not found!" << std::endl;
    }


    void fat32_mkdir(const char* name) {
        // 1. Находим и занимаем кластер
        uint32_t new_cluster = fat32_find_free_cluster();
        if (new_cluster == 0) return;
        fat32_update_fat(new_cluster, 0x0FFFFFFF); // Mark as EOF
        
        // 2. ЗАНУЛЯЕМ сектор этого кластера (критически важно!)
        uint8_t zero_buf[512];
        for(int i=0; i<512; i++) zero_buf[i] = 0;
        Storage::disk_mgr.write_sector(1, get_sector_by_cluster(new_cluster), zero_buf);
        
        // 3. Создаем запись в текущей директории
        uint8_t dir_buf[512];
        uint32_t current_sector = get_sector_by_cluster(current_dir_cluster);
        Storage::disk_mgr.read_sector(1, current_sector, dir_buf);
        
        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)dir_buf;
        for (int i = 0; i < 16; i++) {
            if (entries[i].name[0] == 0x00 || (uint8_t)entries[i].name[0] == 0xE5) {
                for(int j=0; j<11; j++) entries[i].name[j] = ' ';
                for(int j=0; j<8 && name[j]; j++) entries[i].name[j] = (name[j]>='a') ? name[j]-32 : name[j];
            
                entries[i].attributes = 0x10; // Directory
                entries[i].cluster_low = (uint16_t)(new_cluster & 0xFFFF);
                entries[i].cluster_high = (uint16_t)((new_cluster >> 16) & 0xFFFF);
                
                Storage::disk_mgr.write_sector(1, current_sector, dir_buf);
                std::cout << "Directory created at cluster " << new_cluster << std::endl;
                return;
            }
        }
    }



    void fat32_cd(const char* path) {
        // Если ввели ".." или "/", возвращаемся в корень (пока так)
        if (std::strcmp(path, "..") == 0 || std::strcmp(path, "/") == 0) {
            current_dir_cluster = 2; 
            std::cout << "Back to root." << std::endl;
            return;
        }

        fat32_dir_entry_t entry;
        if (fat32_find_file(path, &entry)) {
            if (entry.attributes & 0x10) {
                current_dir_cluster = ((uint32_t)entry.cluster_high << 16) | entry.cluster_low;
            } else {
                std::cerr << "Not a directory!" << std::endl;
            }
        } else {
            std::cerr << "Directory not found!" << std::endl;
        }
    }

    void fat32_rm(const char* filename) {
        uint8_t buffer[512];
        uint32_t sector = get_sector_by_cluster(current_dir_cluster);
        disk_mgr.read_sector(1, sector, buffer);
        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)buffer;

        for (int i = 0; i < 16; i++) {
            if (fat_name_match(filename, entries[i].name)) {
                entries[i].name[0] = 0xE5; // Метка "удалено"
                disk_mgr.write_sector(1, sector, buffer);
                std::cout << "Removed: " << filename << std::endl;
                return;
            }
        }
        std::cerr << "File not found!" << std::endl;
    }

    uint32_t get_sector_by_cluster(uint32_t cluster) {
        return data_start_sector + (cluster - 2) * sectors_per_cluster_global;
    }


    void fat32_touch(const char* name) {
        uint8_t buffer[512];
        uint32_t sector = get_sector_by_cluster(current_dir_cluster);
        disk_mgr.read_sector(1, sector, buffer);
        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)buffer;

        for (int i = 0; i < 16; i++) {
            if (entries[i].name[0] == 0x00 || (uint8_t)entries[i].name[0] == 0xE5) {
                for(int j=0; j<11; j++) entries[i].name[j] = ' ';
                for(int j=0; j<8 && name[j] != '\0' && name[j] != '.'; j++) entries[i].name[j] = name[j];

                entries[i].attributes = 0x20; // File
                entries[i].cluster_low = 0;
                entries[i].cluster_high = 0;
                entries[i].file_size = 0;

                disk_mgr.write_sector(1, sector, buffer);
                std::cout << "File created: " << name << std::endl;
                return;
            }
        }
    }




    uint32_t fat32_find_free_cluster() {
        uint32_t fat_table[128]; // Буфер для одного сектора FAT (512 байт / 4)
        
        // Сканируем первые несколько секторов FAT
        for (uint32_t i = 0; i < 50; i++) {
            Storage::disk_mgr.read_sector(1, fat_start_sector + i, (uint8_t*)fat_table);
            for (int j = 0; j < 128; j++) {
                if (fat_table[j] == 0x00000000) {
                    uint32_t cluster = i * 128 + j;
                    if (cluster >= 2) return cluster; // Кластеры 0 и 1 зарезервированы
                }
            }
        }
        return 0;
    }


    void fat32_update_fat(uint32_t cluster, uint32_t value) {
        uint32_t fat_table[128];
        uint32_t sector = fat_start_sector + (cluster / 128);
        uint32_t offset = cluster % 128;

        Storage::disk_mgr.read_sector(1, sector, (uint8_t*)fat_table);
        fat_table[offset] = value;
        Storage::disk_mgr.write_sector(1, sector, (uint8_t*)fat_table);
    }



}

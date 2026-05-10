#include "../include/fat32.hpp"
#include "../include/ata.hpp"
#include "../include/iostream.hpp"
#include "../include/string.hpp"

namespace Storage {
    uint32_t fat_start_sector = 0;
    uint32_t data_start_sector = 0;
    uint32_t root_dir_sector = 0;
    uint8_t  sectors_per_cluster_global = 0;
    uint32_t current_dir_cluster = 2;
    char current_path[256] = "/";

    void fat32_init(uint8_t drive_idx) {
        uint8_t buffer[512];

        if (!disk_mgr.read_sector(drive_idx, 0, buffer)) {
            std::cerr << "[FAT32] Read Error!" << std::endl;
            return;
        }

        // Сигнатура 0xAA55 в конце сектора
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

        // Сохраняем важные глобальные переменные
        sectors_per_cluster_global = bpb->sectors_per_cluster;

        // Начальный сектор корневой директории
        root_dir_sector = data_start_sector + (bpb->root_cluster - 2) * sectors_per_cluster_global;

        // Инициализация текущего каталога – корень
        current_dir_cluster = bpb->root_cluster;  // обычно 2
        current_path[0] = '/';
        current_path[1] = '\0';

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
        uint32_t sector = get_sector_by_cluster(current_dir_cluster);
        disk_mgr.read_sector(1, sector, buffer);

        fat32_dir_entry_t* entry = (fat32_dir_entry_t*)buffer;

        // std::cout << "Directory listing for cluster " << current_dir_cluster << ":" << std::endl; // DEBUG
        for (int i = 0; i < 16; i++) {
            if (entry[i].name[0] == 0x00) break;
            if ((uint8_t)entry[i].name[0] == 0xE5) continue;

            for (int j = 0; j < 11; j++) std::cout << entry[i].name[j];

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

            if (first_byte != 0 && first_byte != 0xE5) {
                // std::cout << "Slot " << i << " is valid. Comparing..." << std::endl;
                if (fat_name_match(name, entries[i].name)) {
                    *out_entry = entries[i];
                    return true;
                }
            }
        }
        return false;
    }

    bool fat_name_match(const char* input, const char* fat_name) {
        for (int i = 0; i < 8; i++) {
            char in_c = input[i];
            if (in_c == '\0' || in_c == '.' || in_c == '\n' || in_c == '\r') {
                for (int k = i; k < 8; k++) {
                    if (fat_name[k] != ' ') return false;
                }
                return true;
            }
            if (in_c >= 'a' && in_c <= 'z') in_c -= 32;
            if (in_c != fat_name[i]) return false;
        }
        return true;
    }

    // Исправленная версия cat: работает с текущим каталогом
    void fat32_cat(const char* filename) {
        uint8_t buffer[512];
        uint32_t sector = get_sector_by_cluster(current_dir_cluster);
        if (!disk_mgr.read_sector(1, sector, buffer)) {
            std::cerr << "Error reading directory!" << std::endl;
            return;
        }
        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)buffer;

        for (int i = 0; i < 16; i++) {
            if (fat_name_match(filename, entries[i].name)) {
                uint32_t cluster = ((uint32_t)entries[i].cluster_high << 16) | entries[i].cluster_low;
                uint32_t sector = get_sector_by_cluster(cluster); // используем формулу с sectors_per_cluster_global

                uint8_t data[512];
                disk_mgr.read_sector(1, sector, data);

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
        fat32_dir_entry_t existing;
        if (fat32_find_file(name, &existing)) {
            std::cerr << "Error: Directory or file already exists!" << std::endl;
            return;
        }

        uint8_t parent_buffer[512];
        uint32_t parent_sector = get_sector_by_cluster(current_dir_cluster);
        if (!Storage::disk_mgr.read_sector(1, parent_sector, parent_buffer)) {
            std::cerr << "Error reading parent directory!" << std::endl;
            return;
        }

        uint32_t new_cluster = fat32_find_free_cluster();
        if (new_cluster == 0) {
            std::cerr << "No free clusters!" << std::endl;
            return;
        }

        fat32_update_fat(new_cluster, 0x0FFFFFFF);

        fat32_dir_entry_t dot_entries[16];
        std::memset(dot_entries, 0, sizeof(dot_entries));

        // "."
        std::memset(dot_entries[0].name, ' ', 11);
        dot_entries[0].name[0] = '.';
        dot_entries[0].attributes = 0x10;
        dot_entries[0].cluster_low = new_cluster & 0xFFFF;
        dot_entries[0].cluster_high = (new_cluster >> 16) & 0xFFFF;

        // ".."
        uint32_t parent_cluster = (current_dir_cluster == 2) ? 0 : current_dir_cluster;
        std::memset(dot_entries[1].name, ' ', 11);
        dot_entries[1].name[0] = '.';
        dot_entries[1].name[1] = '.';
        dot_entries[1].attributes = 0x10;
        dot_entries[1].cluster_low = parent_cluster & 0xFFFF;
        dot_entries[1].cluster_high = (parent_cluster >> 16) & 0xFFFF;

        if (!Storage::disk_mgr.write_sector(1, get_sector_by_cluster(new_cluster), (uint8_t*)dot_entries)) {
            std::cerr << "Error writing new directory cluster!" << std::endl;
            return;
        }

        fat32_dir_entry_t* parent_entries = (fat32_dir_entry_t*)parent_buffer;
        for (int i = 0; i < 16; i++) {
            if (parent_entries[i].name[0] == 0x00 || (uint8_t)parent_entries[i].name[0] == 0xE5) {
                std::memset(parent_entries[i].name, ' ', 11);
                int j;
                for (j = 0; j < 8 && name[j] != '\0' && name[j] != '.'; j++) {
                    char c = name[j];
                    if (c >= 'a' && c <= 'z') c -= 32;
                    parent_entries[i].name[j] = c;
                }
                parent_entries[i].attributes = 0x10;
                parent_entries[i].cluster_low = new_cluster & 0xFFFF;
                parent_entries[i].cluster_high = (new_cluster >> 16) & 0xFFFF;
                parent_entries[i].file_size = 0;

                if (!Storage::disk_mgr.write_sector(1, parent_sector, parent_buffer)) {
                    std::cerr << "Error writing parent directory!" << std::endl;
                    return;
                }
                // std::cout << "Directory created: " << name << " (cluster " << new_cluster << ")" << std::endl; // DEBUG
                return;
            }
        }
        std::cerr << "No free entry in parent directory!" << std::endl;
    }

    // Полностью переписанный cd: поддержка абсолютных/относительных путей, "..", "."
    void fat32_cd(const char* path) {
        uint32_t old_cluster = current_dir_cluster;
        char old_path[256];
        std::strcpy(old_path, current_path);

        // Если путь абсолютный – начинаем с корня
        if (path[0] == '/') {
            current_dir_cluster = 2;
            current_path[0] = '/';
            current_path[1] = '\0';
            if (path[1] == '\0') return;
            path++;
        }

        char component[256];
        while (*path != '\0') {
            // Пропускаем слеши в начале/между компонентами
            while (*path == '/') path++;
            if (*path == '\0') break;

            // Копируем очередной компонент
            int i = 0;
            while (path[i] != '\0' && path[i] != '/') {
                component[i] = path[i];
                i++;
            }
            component[i] = '\0';
            path += i;

            // Обработка ".."
            if (std::strcmp(component, "..") == 0) {
                uint32_t parent = fat32_get_parent_cluster();
                if (parent != 0) {
                    current_dir_cluster = parent;
                    int len = std::strlen(current_path);
                    if (len > 1) {
                        int last_slash = len - 1;
                        while (last_slash >= 0 && current_path[last_slash] != '/')
                            last_slash--;
                        if (last_slash == 0)
                            current_path[1] = '\0';   // остаёмся в корне
                        else
                            current_path[last_slash] = '\0';
                    }
                }
                continue;
            }

            // "." – остаёмся на месте
            if (std::strcmp(component, ".") == 0) {
                continue;
            }

            // Поиск компонента в текущем каталоге
            fat32_dir_entry_t entry;
            if (!fat32_find_file(component, &entry)) {
                // Откат при ошибке
                current_dir_cluster = old_cluster;
                std::strcpy(current_path, old_path);
                std::cerr << "Directory not found: " << component << std::endl;
                return;
            }

            if (!(entry.attributes & 0x10)) {
                current_dir_cluster = old_cluster;
                std::strcpy(current_path, old_path);
                std::cerr << component << " is not a directory!" << std::endl;
                return;
            }

            // Переходим в найденный каталог
            uint32_t new_cluster = ((uint32_t)entry.cluster_high << 16) | entry.cluster_low;
            current_dir_cluster = new_cluster;

            // Дописываем компонент к текущему пути
            if (current_path[std::strlen(current_path) - 1] != '/')
                std::strcat(current_path, "/");
            std::strcat(current_path, component);
        }
    }



    void fat32_rm(const char* filename) {
        uint8_t buffer[512];
        uint32_t sector = get_sector_by_cluster(current_dir_cluster);
        disk_mgr.read_sector(1, sector, buffer);
        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)buffer;

        for (int i = 0; i < 16; i++) {
            if (fat_name_match(filename, entries[i].name)) {
                entries[i].name[0] = 0xE5;
                disk_mgr.write_sector(1, sector, buffer);
                // std::cout << "Removed: " << filename << std::endl;
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
        Storage::disk_mgr.read_sector(1, sector, buffer);
        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)buffer;

        for (int i = 0; i < 16; i++) {
            if (entries[i].name[0] == 0x00 || (uint8_t)entries[i].name[0] == 0xE5) {
                for (int j = 0; j < 11; j++) entries[i].name[j] = ' ';
                for (int j = 0; j < 8 && name[j] != '\0' && name[j] != '.'; j++) {
                    char c = name[j];
                    if (c >= 'a' && c <= 'z') c -= 32;
                    entries[i].name[j] = c;
                }

                entries[i].attributes = 0x20;
                entries[i].cluster_low = 0;
                entries[i].cluster_high = 0;
                entries[i].file_size = 0;

                Storage::disk_mgr.write_sector(1, sector, buffer);
                // std::cout << "File created: " << name << std::endl;
                return;
            }
        }
    }

    uint32_t fat32_find_free_cluster() {
        uint32_t fat_table[128];
        for (uint32_t i = 0; i < 100; i++) {
            if (!disk_mgr.read_sector(1, fat_start_sector + i, (uint8_t*)fat_table)) return 0;
            for (int j = 0; j < 128; j++) {
                if (fat_table[j] == 0x00000000) {
                    uint32_t cluster = i * 128 + j;
                    if (cluster >= 2) return cluster;
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

    uint32_t fat32_get_parent_cluster() {
        if (current_dir_cluster == 2)
            return 0;

        uint8_t buffer[512];
        uint32_t sector = get_sector_by_cluster(current_dir_cluster);
        if (!disk_mgr.read_sector(1, sector, buffer))
            return 0;

        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)buffer;
        if ((uint8_t)entries[1].name[0] != '.')
            return 0;

        return ((uint32_t)entries[1].cluster_high << 16) | entries[1].cluster_low;
    }

    void fat32_pwd() {
        std::cout << current_path << std::endl;
    }
}
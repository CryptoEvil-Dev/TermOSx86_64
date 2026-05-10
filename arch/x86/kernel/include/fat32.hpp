#pragma once
#include <stdint.h>

namespace Storage {
    // Структура BPB (BIOS Parameter Block) для FAT32
    struct fat32_bpb_t {
        uint8_t  boot_jmp[3];
        char     oem_name[8];
        uint16_t bytes_per_sector;
        uint8_t  sectors_per_cluster;
        uint16_t reserved_sectors;
        uint8_t  fat_count;
        uint16_t root_dir_entries; // Для FAT32 должно быть 0
        uint16_t total_sectors_16;
        uint8_t  media_type;
        uint16_t fat_size_16;      // Для FAT32 должно быть 0
        uint16_t sectors_per_track;
        uint16_t head_count;
        uint32_t hidden_sectors;
        uint32_t total_sectors_32;

        // FAT32 Extended Fields
        uint32_t fat_size_32;
        uint16_t ext_flags;
        uint16_t fs_version;
        uint32_t root_cluster;
        uint16_t fs_info_sector;
        uint16_t backup_boot_sector;
        uint8_t  reserved[12];
        uint8_t  drive_number;
        uint8_t  reserved1;
        uint8_t  boot_signature;
        uint32_t volume_id;
        char     volume_label[11];
        char     system_id[8];
    } __attribute__((packed));

    struct fat32_dir_entry_t {
        char     name[11];      // 8 символов имя + 3 расширение
        uint8_t  attributes;    // Директория, скрытый, системный и т.д.
        uint8_t  reserved;
        uint8_t  created_time_ms;
        uint16_t created_time;
        uint16_t created_date;
        uint16_t last_access_date;
        uint16_t cluster_high;  // Старшие 16 бит номера кластера
        uint16_t modified_time;
        uint16_t modified_date;
        uint16_t cluster_low;   // Младшие 16 бит номера кластера
        uint32_t file_size;
    } __attribute__((packed));


    static uint32_t fat_start_sector;
    static uint32_t data_start_sector;
    static uint32_t root_dir_sector;
    static uint8_t  sectors_per_cluster_global;

    static uint32_t current_dir_cluster = 2; // По умолчанию корень
    static char current_path[256] = "/";
    uint32_t get_sector_by_cluster(uint32_t cluster);

    uint32_t fat32_find_free_cluster();
    void fat32_update_fat(uint32_t cluster, uint32_t value);

    void fat32_init(uint8_t drive_idx);


    void fat32_ls();
    bool fat32_find_file(const char* name, fat32_dir_entry_t* out_entry);
    void fat32_cat(const char* filename);
    void fat32_mkdir(const char* name);
    void fat32_cd(const char* path);
    void fat32_rm(const char* filename);
    void fat32_touch(const char* name);
    bool fat_name_match(const char* input, const char* fat_name);
    
}
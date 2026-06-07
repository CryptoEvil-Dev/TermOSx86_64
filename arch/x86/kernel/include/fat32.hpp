#pragma once
#include <stdint.h>

namespace Storage {
    // Структура BPB (BIOS Parameter Block) для FAT32
    struct fat32_bpb_t {
        uint8_t  boot_jmp[3];           // Код перехода на загрузчик (обычно jmp short)
        char     oem_name[8];           // Имя производителя / системы
        uint16_t bytes_per_sector;      // Байт на сектор (обычно 512)
        uint8_t  sectors_per_cluster;   // Секторов в одном кластере
        uint16_t reserved_sectors;      // Зарезервированные секторы (перед FAT)
        uint8_t  fat_count;             // Количество таблиц FAT (обычно 2)
        uint16_t root_dir_entries;      // Для FAT32 должно быть 0
        uint16_t total_sectors_16;      // Общее число секторов (16-битное), если 0 — см. total_sectors_32
        uint8_t  media_type;            // Тип носителя (0xF8 для жёсткого диска)
        uint16_t fat_size_16;           // Устаревшее поле размера FAT (для FAT32 = 0)
        uint16_t sectors_per_track;     // Секторов на дорожку (геометрия диска)
        uint16_t head_count;            // Устаревшее поле размера FAT (для FAT32 = 0)
        uint32_t hidden_sectors;        // Скрытые секторы перед разделом
        uint32_t total_sectors_32;      // Общее число секторов (если 16-битное = 0)

        // FAT32 Extended Fields
        uint32_t fat_size_32;           // Размер одной FAT в секторах
        uint16_t ext_flags;             // Флаги (биты 0-3 — активная FAT, и т.д.)
        uint16_t fs_version;            // Версия файловой системы (обычно 0)
        uint32_t root_cluster;          // Номер первого кластера корневого каталога
        uint16_t fs_info_sector;        // Сектор с FS Info (обычно 1)
        uint16_t backup_boot_sector;    // Сектор резервной копии загрузчика
        uint8_t  reserved[12];          // Зарезервировано (не используется)
        uint8_t  drive_number;          // Номер диска (0x00 для дискеты, 0x80 для HDD)
        uint8_t  reserved1;             // Зарезервировано
        uint8_t  boot_signature;        // Сигнатура загрузочного сектора (0x28 или 0x29)
        uint32_t volume_id;             // Серийный номер тома
        char     volume_label[11];      // Метка тома (11 символов, как в директории)
        char     system_id[8];          // Идентификатор файловой системы ("FAT32   ")
    } __attribute__((packed));

    // =========================================================================
    // Запись каталога FAT32 (Directory Entry)
    // Описывает файл, поддиректорию или метку тома.
    // =========================================================================
    struct fat32_dir_entry_t {
        char     name[11];          // Имя файла в формате 8.3 (8 символов имя + 3 расширения)
        uint8_t  attributes;        // Атрибуты: 0x10 — директория, 0x08 — метка тома, 0x20 — архивный и т.д.
        uint8_t  reserved;          // Зарезервировано
        uint8_t  created_time_ms;   // Миллисекунды времени создания (0..199)
        uint16_t created_time;      // Время создания (часы, минуты, секунды*2)
        uint16_t created_date;      // Дата создания
        uint16_t last_access_date;  // Дата последнего доступа
        uint16_t cluster_high;      // Старшие 16 бит номера кластера
        uint16_t modified_time;     // Время последнего изменения
        uint16_t modified_date;     // Дата последнего изменения
        uint16_t cluster_low;       // Младшие 16 бит номера первого кластера
        uint32_t file_size;         // Размер файла в байтах (для директорий 0)
    } __attribute__((packed));

    // =========================================================================
    // Глобальные переменные, инициализируемые при вызове fat32_init
    // =========================================================================
    extern uint32_t fat_start_sector;               // Начальный сектор области FAT (LBA)
    extern uint32_t data_start_sector;              // Начальный сектор области данных (кластер 2)
    extern uint32_t root_dir_sector;                // Сектор начала корневого каталога (вычисляется из root_cluster)
    extern uint8_t  sectors_per_cluster_global;     // Сохранённое значение sectors_per_cluster для быстрого доступа
    extern uint32_t current_dir_cluster;            // Номер кластера текущего рабочего каталога

    extern char current_path[256];                  // Текущий путь в текстовом виде (для pwd и навигации)

    // Преобразовать номер кластера (начиная с 2) в LBA-сектор области данных
    uint32_t get_sector_by_cluster(uint32_t cluster);

    // Найти свободный кластер (со значением 0x00000000 в FAT)
    uint32_t fat32_find_free_cluster();

    // Записать значение value в ячейку FAT для заданного кластера
    void fat32_update_fat(uint32_t cluster, uint32_t value);

    // Инициализация — чтение BPB, расчёт глобальных указателей, установка корневого каталога
    void fat32_init(uint8_t drive_idx);


    void fat32_ls();
    bool fat32_find_file(const char* name, fat32_dir_entry_t* out_entry);
    void fat32_cat(const char* filename);
    void fat32_mkdir(const char* name);
    void fat32_cd(const char* path);
    void fat32_pwd();
    void fat32_rm(const char* filename);
    void fat32_touch(const char* name);
    // Сравнить введённое имя с именем в формате FAT, с учётом пробелов и регистр
    bool fat_name_match(const char* input, const char* fat_name);

    uint32_t fat32_get_parent_cluster();
    
}
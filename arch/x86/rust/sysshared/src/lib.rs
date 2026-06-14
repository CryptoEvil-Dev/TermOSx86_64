#![no_std]

#[derive(Copy, Clone)]
#[repr(C, packed)]
pub struct MemoryMapEntry {
    pub base_address: u64,
    pub length: u64,
    pub entry_type: u32,
    pub acpi_extended: u32,
}

#[derive(Copy, Clone)]
#[repr(C, packed)]
pub struct BootInfo {
    // Видеосистема VBE
    pub framebuffer_addr: u32,              // Смещение 0: Физический адрес памяти экрана
    pub screen_width: u16,                  // Смещение 4: 1280
    pub screen_height: u16,                 // Смещение 6: 1024
    pub bpp: u8,                            // Смещение 8: Бит на пиксель (32)
    pub pitch: u16,                         // Смещение 9: Количество байт в одной строке экрана (32 - 5120, 16 - 2560)

    // Системные данные
    pub boot_drive: u8,                     // Смещение 11
    pub memory_map_count: u16,              // Смещение 12
    pub memory_map: [MemoryMapEntry; 32],   // Смещение 14
}

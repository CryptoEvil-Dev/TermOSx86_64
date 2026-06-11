#![no_std]

#[repr(C, packed)]
pub struct MemoryMapEntry {
    pub base_address: u64,
    pub length: u64,
    pub entry_type: u32,
    pub acpi_extended: u32,
}

#[repr(C, packed)]
pub struct BootInfo {
    pub boot_drive: u8,
    pub reserved: u8,
    pub memory_map_count: u16,
    pub framebuffer_addr: u32,
    pub memory_map: [MemoryMapEntry; 32],
}

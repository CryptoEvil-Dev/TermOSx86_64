#![no_std]
#![no_main]

use core::panic::PanicInfo;
use sysshared::BootInfo;
use drivers::FramebufferConsole;

// Импортируем макросы из sysshared
use sysshared::{kprint, kprintln};

const VGA_FONT: &[u8; 4096] = include_bytes!("vga_font.bin");

#[no_mangle]
pub extern "C" fn _kboot() -> ! {
    let boot_info = unsafe { &*(0x0500 as *const BootInfo) };

    // Копируем значения из упакованной структуры в локальные выровненные переменные
    let boot_drive = boot_info.boot_drive;
    let width = boot_info.screen_width;
    let height = boot_info.screen_height;
    let bpp = boot_info.bpp;
    let fb_addr = boot_info.framebuffer_addr;
    let mmap_count = boot_info.memory_map_count;

    let mut console = FramebufferConsole::new(boot_info, VGA_FONT);
    console.clear(); 

    // Теперь передаем в макрос безопасные локальные переменные
    kprintln!(console, "TERMOS Secondary Bootloader Successfully Started!");
    kprintln!(console, "-------------------------------------------------");
    kprintln!(console, "Boot Drive ID : 0x{:X}", boot_drive);
    kprintln!(console, "Screen Size   : {}x{} @ {}bpp", width, height, bpp);
    kprintln!(console, "Framebuffer   : 0x{:X}", fb_addr);
    kprintln!(console, "Memory Map Cnt: {}", mmap_count);
    kprintln!(console, "");

    console.write_str("kernel_shell ");

    loop {
        console.update_cursor();
    }
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! { loop {} }

#![no_std]
#![no_main]

use core::panic::PanicInfo;
use sysshared::BootInfo; // Используем тип из общего модуля!

#[no_mangle]
pub extern "C" fn _kboot() -> ! {
    let boot_info = unsafe { &*(0x0500 as *const BootInfo) };
    
    let lfb_base = boot_info.framebuffer_addr;
    let width = boot_info.screen_width as u32;
    let height = boot_info.screen_height as u32;
    let pitch = boot_info.pitch as u32;
    let bpp = boot_info.bpp;

    unsafe {
        for y in 0..height {
            // Вычисляем начало текущей строки в байтах
            let row_offset = y * pitch;

            for x in 0..width {
                if bpp == 32 {
                    // Честный 32-битный режим (4 байта на пиксель)
                    let pixel_ptr = (lfb_base + row_offset + (x * 4)) as *mut u32;
                    pixel_ptr.write_volatile(0x00003300); // ARGB (Тёмно-зелёный)
                } else if bpp == 16 {
                    // 16-битный режим (2 байта на пиксель, RGB565)
                    let pixel_ptr = (lfb_base + row_offset + (x * 2)) as *mut u16;
                    // В RGB565 тёмно-зелёный цвет (0, 25, 0) кодируется как 0x0320
                    pixel_ptr.write_volatile(0x0320); 
                } else if bpp == 24 {
                    // 24-битный режим (3 байта на пиксель)
                    let pixel_ptr = (lfb_base + row_offset + (x * 3)) as *mut u8;
                    pixel_ptr.write_volatile(0x00); // B
                    pixel_ptr.add(1).write_volatile(0x33); // G
                    pixel_ptr.add(2).write_volatile(0x00); // R
                }
            }
        }
    } 

    loop {}
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

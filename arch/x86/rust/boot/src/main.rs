#![no_std]
#![no_main]

use core::panic::PanicInfo;
use sysshared::BootInfo; // Используем тип из общего модуля!

#[no_mangle]
pub extern "C" fn _kboot() -> ! {
    // Получаем доступ к структурам из 16-битного режима
    let boot_info = unsafe { &*(0x0500 as *const BootInfo) };

    // Адрес экрана из VBE
    let lfb = boot_info.framebuffer_addr as *mut u32;

    unsafe {
        // Заливаем экран темно-зеленым в знак победы!
        for i in 0..(1024 * 768) {
            *lfb.add(i) = 0x003300;
        }
    }

    loop {}
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

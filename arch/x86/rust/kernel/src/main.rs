#![no_std]
#![no_main] // Сообщаем компилятору, что стандартной функции main не будет

use core::panic::PanicInfo;

#[no_mangle]
pub extern "C" fn _start_kernel() -> ! {
    // Точка входа в ваше ядро
    loop {}
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

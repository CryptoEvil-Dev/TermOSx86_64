#![no_std]

use core::fmt;

use sysshared::BootInfo;


// const VGA_FONT: &[u8; 4096] = include_bytes!("vga_font.bin");

#[derive(Copy, Clone)]
pub struct Color(pub u32);

impl Color {
    pub const WHITE: Color = Color(0x00FFFFFF);
    pub const GREEN: Color = Color(0x00003300);
    pub const BLACK: Color = Color(0x00000000);
}


impl fmt::Write for FramebufferConsole {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        let bytes = s.as_bytes();
        for i in 0..bytes.len() {
            self.write_char(bytes[i] as char);
        }

        Ok(())
    }
}


pub struct FramebufferConsole {
    boot_info: &'static BootInfo,
    font_data: &'static [u8; 4096], // Добавили поле для шрифта
    cursor_x: u32,
    cursor_y: u32,
    text_color: Color,
    bg_color: Color,

    cursor_visible: bool,
    cursor_ticks: u32,
    cursor_char: char,
}

impl FramebufferConsole {
    pub fn new(boot_info: &'static BootInfo, font_data: &'static [u8; 4096]) -> Self {
        Self {
            boot_info,
            font_data, // Сохраняем указатель
            cursor_x: 0,
            cursor_y: 0,
            text_color: Color::WHITE,
            bg_color: Color::BLACK,
            cursor_visible: true,
            cursor_ticks: 0,
            cursor_char: '>',
        }
    }


   #[inline(always)]
    fn draw_pixel(&self, x: u32, y: u32, color: Color) {
        if x >= self.boot_info.screen_width as u32 || y >= self.boot_info.screen_height as u32 {
            return;
        }
        
        // Считаем точный сдвиг строки в байтах, используя pitch, переданный из BIOS
        let row_offset = y * self.boot_info.pitch as u32;

        unsafe {
            if self.boot_info.bpp == 32 {
                // Честный 32-битный режим (4 байта на пиксель)
                let ptr = (self.boot_info.framebuffer_addr + row_offset + (x * 4)) as *mut u32;
                ptr.write_volatile(color.0);
            } else if self.boot_info.bpp == 16 {
                // 16-битный режим (2 байта на пиксель, RGB565)
                // Конвертируем 32-битный цвет ARGB в 16-битный формат 5-6-5
                let r = ((color.0 >> 16) & 0xFF) >> 3;
                let g = ((color.0 >> 8) & 0xFF) >> 2;
                let b = (color.0 & 0xFF) >> 3;
                let color_16 = ((r << 11) | (g << 5) | b) as u16;

                let ptr = (self.boot_info.framebuffer_addr + row_offset + (x * 2)) as *mut u16;
                ptr.write_volatile(color_16);
            } else if self.boot_info.bpp == 24 {
                // 24-битный режим (3 байта на пиксель, BGR)
                let ptr = (self.boot_info.framebuffer_addr + row_offset + (x * 3)) as *mut u8;
                ptr.write_volatile((color.0 & 0xFF) as u8);        // Blue
                ptr.add(1).write_volatile(((color.0 >> 8) & 0xFF) as u8);  // Green
                ptr.add(2).write_volatile(((color.0 >> 16) & 0xFF) as u8); // Red
            }
        }
    }


    pub fn draw_char_at(&self, x: u32, y: u32, ch: char, fg: Color, bg: Color) {
        let ascii = (ch as usize) & 0xFF;
        if ascii >= 256 { return; }
        
        let font_offset = ascii * 16;

        for row in 0..16 {
            let bits = self.font_data[font_offset + row];
            for col in 0..8 {
                // Чистая битовая маска: 1 - цвет текста, 0 - цвет фона
                let color = if (bits & (0x80 >> col)) != 0 { fg } else { bg };
                self.draw_pixel(x + col, y + row as u32, color);
            }
        }
    }


    pub fn clear(&self) {
        // Красим экран строго попиксельно через наш обновленный draw_pixel
        for y in 0..(self.boot_info.screen_height as u32) {
            for x in 0..(self.boot_info.screen_width as u32) {
                self.draw_pixel(x, y, self.bg_color);
            }
        }
    }


    /// Безопасный bare-metal скроллинг без вызовов memmove/memcpy
    fn scroll(&mut self) {
        let width = self.boot_info.screen_width as u32;
        let height = self.boot_info.screen_height as u32;
        
        unsafe {
            // Переносим каждую строку (начиная с 16-й) на 16 пикселей вверх
            for y in 16..height {
                let dest_row_offset = (y - 16) * self.boot_info.pitch as u32;
                let src_row_offset = y * self.boot_info.pitch as u32;

                for x in 0..width {
                    let src_ptr = (self.boot_info.framebuffer_addr + src_row_offset + (x * 4)) as *const u32;
                    let dest_ptr = (self.boot_info.framebuffer_addr + dest_row_offset + (x * 4)) as *mut u32;
                    dest_ptr.write_volatile(src_ptr.read_volatile());
                }
            }

            // Очищаем освободившуюся нижнюю зону (последние 16 строк) цветом фона
            for y in (height - 16)..height {
                for x in 0..width {
                    self.draw_pixel(x, y, self.bg_color);
                }
            }
        }
        self.cursor_y -= 16;
    }

    pub fn new_line(&mut self) {
        self.draw_char_at(self.cursor_x, self.cursor_y, ' ', self.bg_color, self.bg_color);

        self.cursor_x = 0;
        self.cursor_y += 16;

        if self.cursor_y + 16 >= self.boot_info.screen_height as u32 {
            self.scroll();
        }
    }

    pub fn write_char(&mut self, ch: char) {
        match ch {
            '\n' => self.new_line(),
            '\r' => self.cursor_x = 0,
            _ => {
                self.draw_char_at(self.cursor_x, self.cursor_y, ch, self.text_color, self.bg_color);
                self.cursor_x += 8;

                if self.cursor_x + 8 >= self.boot_info.screen_width as u32 {
                    self.new_line();
                }
            }
        }
    }

    pub fn write_str(&mut self, s: &str) {

        let bytes = s.as_bytes();

        for i in 0..bytes.len() {
            self.write_char(bytes[i] as char);
        }
        self.render_cursor();
    }

    fn render_cursor(&self) {
        let color = if self.cursor_visible { self.text_color } else { self.bg_color };
        self.draw_char_at(self.cursor_x, self.cursor_y, self.cursor_char, color, self.bg_color);
    }

    pub fn update_cursor(&mut self) {
        self.cursor_ticks += 1;
        if self.cursor_ticks >= 200_000_000 { // Подняли лимит, так как в 1280x1024 циклы работают быстрее
            self.cursor_ticks = 0;
            self.cursor_visible = !self.cursor_visible;
            self.render_cursor();
        }
    }
}

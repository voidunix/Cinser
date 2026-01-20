#include "video.h"
#include "multiboot.h"

static uint32_t* vbe_buffer = 0;
static uint32_t vbe_pitch = 0;

static void vesa_init_impl(void* info) {
    multiboot_info_t* mbi = (multiboot_info_t*)info;
    vbe_buffer = (uint32_t*)(uintptr_t)mbi->framebuffer_addr;
    vbe_pitch = mbi->framebuffer_pitch;
    
    // Atualiza as informacoes da struct do driver
    extern video_driver_t vesa_driver;
    vesa_driver.width = mbi->framebuffer_width;
    vesa_driver.height = mbi->framebuffer_height;
}

static void vesa_put_pixel(int x, int y, uint32_t color) {
    if (!vbe_buffer) return;
    extern video_driver_t vesa_driver;
    if (x < 0 || y < 0) return;
    if (x >= vesa_driver.width || y >= vesa_driver.height) return;
    // Pitch vem em bytes, buffer é uint32 (4 bytes). Divisao por 4 necessaria.
    unsigned offset = (y * (vbe_pitch/4)) + x;
    vbe_buffer[offset] = color;
}

static void vesa_clear(uint32_t color) {
    extern video_driver_t vesa_driver;
    if (!vbe_buffer) return;
    if (!vbe_pitch) return;
    unsigned stride = (unsigned)(vbe_pitch / 4u);
    for (int y = 0; y < vesa_driver.height; y++) {
        uint32_t *row = vbe_buffer + (y * stride);
        for (int x = 0; x < vesa_driver.width; x++) {
            row[x] = color;
        }
    }
}

static void vesa_update(void) {
    // Nada para fazer, VESA escreve direto na memória
}

// Definição do Driver
video_driver_t vesa_driver = {
    .driver_name = "VESA VBE",
    .width = 0,
    .height = 0,
    .bpp = 32,
    .init = vesa_init_impl,
    .put_pixel = vesa_put_pixel,
    .clear_screen = vesa_clear,
    .update = vesa_update
};
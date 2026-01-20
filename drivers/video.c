#include "video.h"
#include "multiboot.h"

// Driver ativo (começa nulo)
video_driver_t* g_video_driver = 0;

// Importa o driver VESA definido no outro arquivo
extern video_driver_t vesa_driver;

void video_init_system(void* mb_ptr) {
    multiboot_info_t* mbi = (multiboot_info_t*)mb_ptr;

    // Se tivermos bit 12 (framebuffer) e magic number correto
    // Carregamos o driver VESA.
    // Futuramente aqui voce colocaria: if (is_intel) ...
    if (mbi->flags & (1 << 12)) {
        g_video_driver = &vesa_driver;
        g_video_driver->init(mb_ptr);
    }
}

// Wrapper: Kernel chama isso, e isso chama o driver certo
void put_pixel(int x, int y, uint32_t color) {
    if (g_video_driver) {
        g_video_driver->put_pixel(x, y, color);
    }
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for(int i=0; i<h; i++) {
        for(int j=0; j<w; j++) {
            put_pixel(x+j, y+i, color);
        }
    }
}
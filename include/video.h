#ifndef VIDEO_H
#define VIDEO_H

#include <stdint.h>

// A Estrutura Universal de Driver
typedef struct {
    const char* driver_name;
    int width;
    int height;
    int bpp;
    
    // Funcoes que o driver deve ter
    void (*init)(void* info);
    void (*put_pixel)(int x, int y, uint32_t color);
    void (*clear_screen)(uint32_t color);
    void (*update)(void);
} video_driver_t;

// Variavel global do driver ativo
extern video_driver_t* g_video_driver;

// Funcoes para o Kernel chamar
void video_init_system(void* multiboot_info);
void put_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);

#endif
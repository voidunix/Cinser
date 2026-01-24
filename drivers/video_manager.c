/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: video.c
 * Descrição: Núcleo do sistema operacional / Gerenciamento de processos.
 * * Copyright (C) 2026 Tervia Corporation.
 *
 * Este programa é um software livre: você pode redistribuí-lo e/ou 
 * modificá-lo sob os termos da Licença Pública Geral GNU como publicada 
 * pela Free Software Foundation, bem como a versão 3 da Licença.
 *
 * Este programa é distribuído na esperança de que possa ser útil, 
 * mas SEM NENHUMA GARANTIA; sem uma garantia implícita de ADEQUAÇÃO 
 * a qualquer MERCADO ou APLICAÇÃO EM PARTICULAR. Veja a 
 * Licença Pública Geral GNU para mais detalhes.
 ****************************************************************************/

#include "video.h"
#include "multiboot.h"

video_driver_t* g_video_driver = 0;
extern video_driver_t vesa_driver; // Referência ao driver que criamos acima

void video_init_system(void* mb_ptr) {
    multiboot_info_t* mbi = (multiboot_info_t*)mb_ptr;

    // Lógica de seleção de driver
    // Futuramente: if (pci_detect_intel()) g_video_driver = &intel_driver;
    
    // Por enquanto, fallback para VESA se o bit 12 (framebuffer) estiver ativo
    if (mbi->flags & (1 << 12)) {
        g_video_driver = &vesa_driver;
        
        // Configura as dimensões baseadas no que o GRUB deu
        vesa_driver.width = mbi->framebuffer_width;
        vesa_driver.height = mbi->framebuffer_height;
        
        // Inicializa o driver específico
        g_video_driver->init(mbi);
    }
}

// Wrapper Genérico: O resto do Kernel só chama isso
void put_pixel(int x, int y, uint32_t color) {
    if (g_video_driver) {
        g_video_driver->put_pixel(x, y, color);
    }
}
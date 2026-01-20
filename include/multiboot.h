#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

/* O número mágico passado pelo bootloader (deve estar em EAX) */
#define MULTIBOOT_BOOTLOADER_MAGIC    0x2BADB002

/* Flags para verificar se o campo está válido */
#define MULTIBOOT_INFO_MEMORY         0x00000001
#define MULTIBOOT_INFO_BOOTDEV        0x00000002
#define MULTIBOOT_INFO_CMDLINE        0x00000004
#define MULTIBOOT_INFO_MODS           0x00000008
#define MULTIBOOT_INFO_AOUT_SYMS      0x00000010
#define MULTIBOOT_INFO_ELF_SHDR       0X00000020
#define MULTIBOOT_INFO_MEM_MAP        0x00000040
#define MULTIBOOT_INFO_DRIVE_INFO     0x00000080
#define MULTIBOOT_INFO_CONFIG_TABLE   0x00000100
#define MULTIBOOT_INFO_BOOT_LOADER_NAME 0x00000200
#define MULTIBOOT_INFO_APM_TABLE      0x00000400
#define MULTIBOOT_INFO_VBE_INFO       0x00000800
#define MULTIBOOT_INFO_FRAMEBUFFER_INFO 0x00001000

/* Estrutura principal que o GRUB entrega em EBX */
typedef struct multiboot_info {
    uint32_t flags;

    // Memória disponível (flag bit 0)
    uint32_t mem_lower;
    uint32_t mem_upper;

    // Disco de boot (flag bit 1)
    uint32_t boot_device;

    // Linha de comando do kernel (flag bit 2)
    uint32_t cmdline;

    // Módulos (flag bit 3)
    uint32_t mods_count;
    uint32_t mods_addr;

    // Tabelas de símbolos (ELF ou a.out)
    union {
        struct {
            uint32_t tabsize;
            uint32_t strsize;
            uint32_t addr;
            uint32_t reserved;
        } aout_sym;
        struct {
            uint32_t num;
            uint32_t size;
            uint32_t addr;
            uint32_t shndx;
        } elf_sec;
    } u;

    // Mapa de Memória (flag bit 6)
    uint32_t mmap_length;
    uint32_t mmap_addr;

    // Drives (flag bit 7)
    uint32_t drives_length;
    uint32_t drives_addr;

    // Tabela de configuração ROM (flag bit 8)
    uint32_t config_table;

    // Nome do Bootloader (flag bit 9)
    uint32_t boot_loader_name;

    // Tabela APM (flag bit 10)
    uint32_t apm_table;

    // Informações de Vídeo VBE (flag bit 11)
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    // FRAMEBUFFER (VESA) INFO (flag bit 12)
    // É AQUI QUE A MÁGICA ACONTECE
    uint64_t framebuffer_addr;  // Endereço Físico onde desenhamos os pixels
    uint32_t framebuffer_pitch; // Bytes por linha
    uint32_t framebuffer_width; // Largura (ex: 1024)
    uint32_t framebuffer_height;// Altura (ex: 768)
    uint8_t framebuffer_bpp;    // Bits por pixel (ex: 32)
    
    uint8_t framebuffer_type;   // 0=Indexado, 1=RGB Direto, 2=Texto
    
    union {
        struct {
            uint32_t framebuffer_palette_addr;
            uint16_t framebuffer_palette_num_colors;
        };
        struct {
            // Posição e tamanho das cores (Red, Green, Blue)
            uint8_t framebuffer_red_field_position;
            uint8_t framebuffer_red_mask_size;
            uint8_t framebuffer_green_field_position;
            uint8_t framebuffer_green_mask_size;
            uint8_t framebuffer_blue_field_position;
            uint8_t framebuffer_blue_mask_size;
        };
    };

} __attribute__((packed)) multiboot_info_t;

// Estrutura para o mapa de memória (mmap)
typedef struct multiboot_mmap_entry {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed)) multiboot_memory_map_t;

#endif
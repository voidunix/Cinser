/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: multiboot.h
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

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

/*
 * Multiboot v1 structures (GRUB legacy / grub-mkrescue normalmente usa isso)
 * Referência: Multiboot Specification v0.6.96
 */

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

// info flags
#define MULTIBOOT_INFO_MEMORY      0x00000001
#define MULTIBOOT_INFO_BOOTDEV     0x00000002
#define MULTIBOOT_INFO_CMDLINE     0x00000004
#define MULTIBOOT_INFO_MODS        0x00000008
#define MULTIBOOT_INFO_AOUT_SYMS   0x00000010
#define MULTIBOOT_INFO_ELF_SHDR    0x00000020
#define MULTIBOOT_INFO_MEM_MAP     0x00000040
#define MULTIBOOT_INFO_DRIVE_INFO  0x00000080
#define MULTIBOOT_INFO_CONFIG_TABLE 0x00000100
#define MULTIBOOT_INFO_BOOT_LOADER_NAME 0x00000200
#define MULTIBOOT_INFO_APM_TABLE   0x00000400
#define MULTIBOOT_INFO_VBE_INFO    0x00000800
#define MULTIBOOT_INFO_FRAMEBUFFER_INFO 0x00001000  // bit 12

typedef struct multiboot_mmap_entry {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed)) multiboot_mmap_entry_t;

typedef struct multiboot_mod_list {
    uint32_t mod_start;
    uint32_t mod_end;
    uint32_t cmdline;
    uint32_t pad;
} multiboot_mod_list_t;

typedef struct multiboot_aout_symbol_table {
    uint32_t tabsize;
    uint32_t strsize;
    uint32_t addr;
    uint32_t reserved;
} multiboot_aout_symbol_table_t;

typedef struct multiboot_elf_section_header_table {
    uint32_t num;
    uint32_t size;
    uint32_t addr;
    uint32_t shndx;
} multiboot_elf_section_header_table_t;

typedef struct multiboot_info {
    uint32_t flags;

    // memory
    uint32_t mem_lower;
    uint32_t mem_upper;

    // boot device
    uint32_t boot_device;

    // command line
    uint32_t cmdline;

    // modules
    uint32_t mods_count;
    uint32_t mods_addr;

    // aout/elf
    union {
        multiboot_aout_symbol_table_t aout_sym;
        multiboot_elf_section_header_table_t elf_sec;
    } u;

    // memory map
    uint32_t mmap_length;
    uint32_t mmap_addr;

    // drives
    uint32_t drives_length;
    uint32_t drives_addr;

    // ROM config table
    uint32_t config_table;

    // boot loader name
    uint32_t boot_loader_name;

    // APM table
    uint32_t apm_table;

    // VBE info (flag bit 11)
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    // Framebuffer info (flag bit 12)
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type; // 0 = indexed, 1 = RGB, 2 = EGA text
    uint16_t reserved;

    // color info
    union {
        struct {
            uint32_t framebuffer_palette_addr;
            uint16_t framebuffer_palette_num_colors;
        } __attribute__((packed)) palette;

        struct {
            uint8_t framebuffer_red_field_position;
            uint8_t framebuffer_red_mask_size;
            uint8_t framebuffer_green_field_position;
            uint8_t framebuffer_green_mask_size;
            uint8_t framebuffer_blue_field_position;
            uint8_t framebuffer_blue_mask_size;
        } __attribute__((packed)) rgb;
    } color_info;
} __attribute__((packed)) multiboot_info_t;

#endif

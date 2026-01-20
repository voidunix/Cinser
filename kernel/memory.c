/****************************************************************************
 * Projeto: Tervia Cinser OS
 * Arquivo: memory.c
 * Descricao: Gerenciador de memoria (PMM + heap do kernel).
 * Copyright (C) 2026 Tervia Corporation.
 *
 * Este programa e um software livre: voce pode redistribui-lo e/ou
 * modifica-lo sob os termos da Licenca Publica Geral GNU como publicada
 * pela Free Software Foundation, bem como a versao 3 da Licenca.
 *
 * Este programa e distribuido na esperanca de que possa ser util,
 * mas SEM NENHUMA GARANTIA; sem uma garantia implicita de ADEQUACAO
 * a qualquer MERCADO ou APLICACAO EM PARTICULAR. Veja a
 * Licenca Publica Geral GNU para mais detalhes.
 ****************************************************************************/

#include <stdint.h>
#include "memory.h"

#define MULTIBOOT_MAGIC 0x2BADB002u

// ----------------------------
// Multiboot v1 (subset)
// ----------------------------

typedef struct {
    uint32_t size;
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t len_low;
    uint32_t len_high;
    uint32_t type;
} __attribute__((packed)) multiboot_mmap_entry_t;

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
} multiboot_info_t;

// Linker fornece fim do kernel
extern uint32_t _kernel_end;

static inline uint32_t align_up_u32(uint32_t v, uint32_t a) {
    return (v + (a - 1u)) & ~(a - 1u);
}

// ----------------------------
// PMM (bitmap de frames 4KiB)
// ----------------------------

#define PAGE_SIZE 4096u
#define PHYS_4G_LIMIT 0x100000000ull

static uint8_t *g_pmm_bitmap = 0;
static uint32_t g_pmm_frames_total = 0;
static uint32_t g_pmm_frames_used = 0;
static uint32_t g_pmm_bitmap_bytes = 0;

static inline void bitmap_set(uint32_t frame) {
    g_pmm_bitmap[frame >> 3] |= (uint8_t)(1u << (frame & 7u));
}
static inline void bitmap_clear(uint32_t frame) {
    g_pmm_bitmap[frame >> 3] &= (uint8_t)~(1u << (frame & 7u));
}
static inline uint8_t bitmap_test(uint32_t frame) {
    return (g_pmm_bitmap[frame >> 3] >> (frame & 7u)) & 1u;
}

static void pmm_mark_all_used(void) {
    for (uint32_t i = 0; i < g_pmm_bitmap_bytes; i++) g_pmm_bitmap[i] = 0xFF;
    g_pmm_frames_used = g_pmm_frames_total;
}

static void pmm_mark_region_free(uint32_t base, uint32_t length) {
    // base/length em bytes
    uint32_t start = (base + PAGE_SIZE - 1u) / PAGE_SIZE;
    uint32_t end   = (base + length) / PAGE_SIZE;
    if (end > g_pmm_frames_total) end = g_pmm_frames_total;
    for (uint32_t f = start; f < end; f++) {
        if (bitmap_test(f)) {
            bitmap_clear(f);
            g_pmm_frames_used--;
        }
    }
}

static void pmm_mark_region_used(uint32_t base, uint32_t length) {
    uint32_t start = base / PAGE_SIZE;
    uint32_t end   = (base + length + PAGE_SIZE - 1u) / PAGE_SIZE;
    if (end > g_pmm_frames_total) end = g_pmm_frames_total;
    for (uint32_t f = start; f < end; f++) {
        if (!bitmap_test(f)) {
            bitmap_set(f);
            g_pmm_frames_used++;
        }
    }
}

uint32_t pmm_alloc_page(void) {
    for (uint32_t f = 0; f < g_pmm_frames_total; f++) {
        if (!bitmap_test(f)) {
            bitmap_set(f);
            g_pmm_frames_used++;
            return f * PAGE_SIZE;
        }
    }
    return 0;
}

void pmm_free_page(uint32_t paddr) {
    uint32_t f = paddr / PAGE_SIZE;
    if (f >= g_pmm_frames_total) return;
    if (bitmap_test(f)) {
        bitmap_clear(f);
        g_pmm_frames_used--;
    }
}

// ----------------------------
// Heap (kmalloc/kfree)
// ----------------------------

typedef struct heap_block {
    uint32_t size;            // payload (bytes)
    uint32_t magic;
    uint8_t  free;
    uint8_t  _pad[3];
    struct heap_block *prev;
    struct heap_block *next;
} heap_block_t;

#define HEAP_MAGIC 0xC15E1234u
#define ALIGN8(x) (((x) + 7u) & ~7u)

static heap_block_t *g_heap_head = 0;
static uint32_t g_heap_start = 0;
static uint32_t g_heap_end = 0;
static uint32_t g_heap_max = 0;

// prefix para alinhado
typedef struct {
    uint32_t magic;
    uint32_t base_ptr; // ponteiro original retornado por kmalloc
} aligned_prefix_t;

#define ALIGNED_MAGIC 0xA11A1100u

static void heap_init(uint32_t heap_base, uint32_t heap_size) {
    g_heap_start = heap_base;
    g_heap_end = heap_base + heap_size;
    g_heap_max = g_heap_end;

    g_heap_head = (heap_block_t*)heap_base;
    g_heap_head->size = heap_size - (uint32_t)sizeof(heap_block_t);
    g_heap_head->magic = HEAP_MAGIC;
    g_heap_head->free = 1;
    g_heap_head->prev = 0;
    g_heap_head->next = 0;
}

static void heap_split(heap_block_t *blk, uint32_t needed) {
    // needed = payload bytes ja alinhados
    uint32_t total_needed = needed + (uint32_t)sizeof(heap_block_t);
    uint32_t blk_total = blk->size + (uint32_t)sizeof(heap_block_t);

    // precisa sobrar espaco suficiente pra outro bloco
    if (blk_total < total_needed + (uint32_t)sizeof(heap_block_t) + 8u) return;

    uint32_t new_addr = (uint32_t)((uint8_t*)blk + (uint32_t)sizeof(heap_block_t) + needed);
    heap_block_t *n = (heap_block_t*)new_addr;
    n->size = (uint32_t)(blk_total - total_needed - (uint32_t)sizeof(heap_block_t));
    n->magic = HEAP_MAGIC;
    n->free = 1;
    n->prev = blk;
    n->next = blk->next;
    if (n->next) n->next->prev = n;

    blk->size = needed;
    blk->next = n;
}

static void heap_coalesce(heap_block_t *blk) {
    // junta com next
    if (blk->next && blk->next->free) {
        heap_block_t *n = blk->next;
        blk->size = blk->size + (uint32_t)sizeof(heap_block_t) + n->size;
        blk->next = n->next;
        if (blk->next) blk->next->prev = blk;
    }
    // junta com prev
    if (blk->prev && blk->prev->free) {
        heap_block_t *p = blk->prev;
        p->size = p->size + (uint32_t)sizeof(heap_block_t) + blk->size;
        p->next = blk->next;
        if (p->next) p->next->prev = p;
    }
}

static heap_block_t* heap_find_fit(uint32_t needed) {
    heap_block_t *cur = g_heap_head;
    while (cur) {
        if (cur->magic != HEAP_MAGIC) return 0; // heap corrompido
        if (cur->free && cur->size >= needed) return cur;
        cur = cur->next;
    }
    return 0;
}

static int heap_grow(uint32_t more_bytes) {
    // cresce heap alocando paginas fisicas e anexando um bloco no fim.
    uint32_t grow = align_up_u32(more_bytes, PAGE_SIZE);
    uint32_t new_end = g_heap_end + grow;

    // aloca paginas fisicas correspondentes (identity)
    for (uint32_t p = g_heap_end; p < new_end; p += PAGE_SIZE) {
        uint32_t got = pmm_alloc_page();
        if (!got) return 0;
        // Mantem identidade: marca a pagina desejada.
        pmm_free_page(got);
        pmm_mark_region_used(p, PAGE_SIZE);
    }

    // anexa novo bloco
    heap_block_t *last = g_heap_head;
    while (last && last->next) last = last->next;

    heap_block_t *n = (heap_block_t*)g_heap_end;
    n->magic = HEAP_MAGIC;
    n->free = 1;
    n->prev = last;
    n->next = 0;
    n->size = grow - (uint32_t)sizeof(heap_block_t);
    if (last) last->next = n;

    g_heap_end = new_end;
    g_heap_max = g_heap_end;

    // tentar coalescer caso ultimo ja seja free
    if (n->prev && n->prev->free) heap_coalesce(n);
    return 1;
}

void* kmalloc(uint32_t size) {
    if (size == 0) return 0;
    uint32_t needed = ALIGN8(size);

    heap_block_t *blk = heap_find_fit(needed);
    if (!blk) {
        if (!heap_grow(needed + (uint32_t)sizeof(heap_block_t))) return 0;
        blk = heap_find_fit(needed);
        if (!blk) return 0;
    }

    heap_split(blk, needed);
    blk->free = 0;
    return (void*)((uint8_t*)blk + (uint32_t)sizeof(heap_block_t));
}

void* kmalloc_aligned(uint32_t size, uint32_t align) {
    if (align < 8u) align = 8u;
    // align deve ser potencia de 2
    if ((align & (align - 1u)) != 0u) align = 8u;

    uint32_t extra = align + (uint32_t)sizeof(aligned_prefix_t);
    uint8_t *base = (uint8_t*)kmalloc(size + extra);
    if (!base) return 0;

    uint32_t addr = (uint32_t)base + (uint32_t)sizeof(aligned_prefix_t);
    uint32_t aligned = (addr + (align - 1u)) & ~(align - 1u);

    aligned_prefix_t *pfx = (aligned_prefix_t*)(aligned - (uint32_t)sizeof(aligned_prefix_t));
    pfx->magic = ALIGNED_MAGIC;
    pfx->base_ptr = (uint32_t)base;

    return (void*)aligned;
}

void kfree(void *ptr) {
    if (!ptr) return;

    // Se foi kmalloc_aligned, desfaz
    aligned_prefix_t *pfx = (aligned_prefix_t*)((uint8_t*)ptr - (uint32_t)sizeof(aligned_prefix_t));
    if (pfx->magic == ALIGNED_MAGIC && pfx->base_ptr != 0) {
        ptr = (void*)pfx->base_ptr;
    }

    heap_block_t *blk = (heap_block_t*)((uint8_t*)ptr - (uint32_t)sizeof(heap_block_t));
    if (blk->magic != HEAP_MAGIC) return;

    blk->free = 1;
    heap_coalesce(blk);
}

// ----------------------------
// Meminfo / stats
// ----------------------------

uint32_t memory_total_kib(void) {
    return (g_pmm_frames_total * PAGE_SIZE) / 1024u;
}

uint32_t memory_used_kib(void) {
    return (g_pmm_frames_used * PAGE_SIZE) / 1024u;
}

uint32_t memory_free_kib(void) {
    return ((g_pmm_frames_total - g_pmm_frames_used) * PAGE_SIZE) / 1024u;
}

static char g_meminfo_buf[96];

static char* u32_to_dec(char *dst, uint32_t v) {
    char tmp[16];
    int n = 0;
    if (v == 0) {
        *dst++ = '0';
        return dst;
    }
    while (v > 0 && n < 15) {
        tmp[n++] = (char)('0' + (v % 10u));
        v /= 10u;
    }
    while (n--) *dst++ = tmp[n];
    return dst;
}

const char* meminfo_str(void) {
    // "Mem: X KiB total, Y KiB used, Z KiB free"
    char *p = g_meminfo_buf;
    const char *a = "Mem: ";
    while (*a) *p++ = *a++;
    p = u32_to_dec(p, memory_total_kib());
    const char *b = " KiB total, ";
    while (*b) *p++ = *b++;
    p = u32_to_dec(p, memory_used_kib());
    const char *c = " KiB used, ";
    while (*c) *p++ = *c++;
    p = u32_to_dec(p, memory_free_kib());
    const char *d = " KiB free";
    while (*d) *p++ = *d++;
    *p = 0;
    return g_meminfo_buf;
}

// ----------------------------
// Init
// ----------------------------

void memory_init(uint32_t multiboot_magic, uint32_t mb_info_ptr) {
    if (multiboot_magic != MULTIBOOT_MAGIC) {
        // sem multiboot: nao inicializa PMM, mas ainda tenta um heap minimo apos o kernel
        uint32_t kend = (uint32_t)&_kernel_end;
        uint32_t heap_base = align_up_u32(kend, 16u);
        heap_init(heap_base, 256u * 1024u);
        return;
    }

    multiboot_info_t *mb = (multiboot_info_t*)mb_info_ptr;

    // precisa de mmap
    if ((mb->flags & (1u << 6)) == 0u) {
        uint32_t kend = (uint32_t)&_kernel_end;
        uint32_t heap_base = align_up_u32(kend, 16u);
        heap_init(heap_base, 256u * 1024u);
        return;
    }

    // Limite REAL de RAM (em bytes): use mem_upper (flags bit0) e trunque mmap.
    // Isso evita "inventar" RAM em VMs pequenas e funciona em PC real tambem.
    uint64_t ram_limit_bytes = 0;
    if ((mb->flags & 1u) != 0u) {
        // mem_upper = KiB acima de 1MiB. Total aproximado = (mem_upper + 1024) KiB.
        ram_limit_bytes = ((uint64_t)mb->mem_upper + 1024ull) * 1024ull;
    }
    if (ram_limit_bytes == 0) {
        // fallback: limite 4GiB (i386 sem PAE)
        ram_limit_bytes = PHYS_4G_LIMIT;
    }
    if (ram_limit_bytes > PHYS_4G_LIMIT) ram_limit_bytes = PHYS_4G_LIMIT;

    g_pmm_frames_total = (uint32_t)(ram_limit_bytes / (uint64_t)PAGE_SIZE);
    if (g_pmm_frames_total < 256u) g_pmm_frames_total = 256u;

    g_pmm_bitmap_bytes = (g_pmm_frames_total + 7u) / 8u;

    // bitmap logo apos o kernel
    uint32_t kend = (uint32_t)&_kernel_end;
    uint32_t bitmap_addr = align_up_u32(kend, 16u);
    g_pmm_bitmap = (uint8_t*)bitmap_addr;

    // Reserva espaco do bitmap
    uint32_t bitmap_end = bitmap_addr + g_pmm_bitmap_bytes;

    // Depois do bitmap, heap inicial
    uint32_t heap_base = align_up_u32(bitmap_end, 16u);
    uint32_t heap_size = 512u * 1024u; // 512 KiB inicial

    // Zera bitmap e marca tudo como usado, depois libera regioes RAM
    pmm_mark_all_used();

    uint32_t mmap_end = mb->mmap_addr + mb->mmap_length;

    // libera apenas tipo 1 (RAM), truncando ao limite real
    for (uint32_t p = mb->mmap_addr; p < mmap_end; ) {
        multiboot_mmap_entry_t *e = (multiboot_mmap_entry_t*)p;

        if (e->type == 1u) {
            uint64_t start = ((uint64_t)e->addr_high << 32) | (uint64_t)e->addr_low;
            uint64_t len   = ((uint64_t)e->len_high  << 32) | (uint64_t)e->len_low;
            uint64_t end   = start + len;

            // corta fora do limite real
            if (start < ram_limit_bytes && end > start) {
                if (end > ram_limit_bytes) end = ram_limit_bytes;

                // i386 sem PAE: ignore acima de 4GiB
                if (start < PHYS_4G_LIMIT) {
                    if (end > PHYS_4G_LIMIT) end = PHYS_4G_LIMIT;
                    if (end > start) {
                        pmm_mark_region_free((uint32_t)start, (uint32_t)(end - start));
                    }
                }
            }
        }

        p += e->size + 4u;
    }

    // reserva regiao baixa (0..1MiB) por seguranca
    pmm_mark_region_used(0u, 0x100000u);

    // reserva kernel + bitmap + heap inicial
    uint32_t reserved_end = heap_base + heap_size;
    if ((uint64_t)reserved_end > ram_limit_bytes) {
        // se RAM for muito pequena, nao deixe estourar o limite
        reserved_end = (uint32_t)ram_limit_bytes;
    }
    if (reserved_end > 0x100000u) {
        pmm_mark_region_used(0x100000u, reserved_end - 0x100000u);
    }

    // init heap
    heap_init(heap_base, heap_size);
}

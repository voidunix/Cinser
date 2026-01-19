#include <stdint.h>

// Handler em ASM (definido em boot/isr_halt.s)
extern void isr_halt(void);

#define IDT_ENTRIES 256

struct __attribute__((packed)) idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
};

struct __attribute__((packed)) idt_ptr {
    uint16_t limit;
    uint32_t base;
};

static struct idt_entry idt[IDT_ENTRIES];

static void idt_set_gate(int n, uint32_t handler, uint16_t sel, uint8_t flags) {
    idt[n].base_low  = (uint16_t)(handler & 0xFFFF);
    idt[n].sel       = sel;
    idt[n].always0   = 0;
    idt[n].flags     = flags;
    idt[n].base_high = (uint16_t)((handler >> 16) & 0xFFFF);
}

void idt_init(void) {
    // 0x08 = nosso code segment (ver boot/boot.s)
    // flags 0x8E = present | ring0 | 32-bit interrupt gate
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, (uint32_t)(uintptr_t)&isr_halt, 0x08, 0x8E);
    }

    struct idt_ptr idtp;
    idtp.limit = (uint16_t)(sizeof(idt) - 1);
    idtp.base  = (uint32_t)(uintptr_t)&idt[0];

    __asm__ volatile("lidt %0" : : "m"(idtp));
}

# Tervia Cinser - i386
# Updated for Video HAL (VESA) support

NAME    := Cinser
BUILD   := build
OBJDIR  := $(BUILD)/obj
ISODIR  := $(BUILD)/isodir

KERNEL  := $(BUILD)/kernel.bin
ISO     := $(BUILD)/$(NAME).iso

CC      := gcc
LD      := ld
AS      := nasm

# Flags de compilação
CFLAGS  := -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -Wall -Wextra -Iinclude \
           -fno-asynchronous-unwind-tables -fno-omit-frame-pointer
LDFLAGS := -m elf_i386 -T boot/linker.ld

# Lista de Objetos (Adicionados video.o e video_vesa.o)
OBJS := \
  $(OBJDIR)/multiboot.o \
  $(OBJDIR)/boot.o \
  $(OBJDIR)/isr_halt.o \
  $(OBJDIR)/interrupts.o \
  $(OBJDIR)/kernel.o \
  $(OBJDIR)/idt.o \
  $(OBJDIR)/isr.o \
  $(OBJDIR)/irq.o \
  $(OBJDIR)/pic.o \
  $(OBJDIR)/cmos.o \
  $(OBJDIR)/memory.o \
  $(OBJDIR)/sysconfig.o \
  $(OBJDIR)/time.o \
  $(OBJDIR)/keyboard.o \
  $(OBJDIR)/video.o \
  $(OBJDIR)/video_vesa.o \
  $(OBJDIR)/console.o

.PHONY: all iso run clean dirs check-tools

all: iso

check-tools:
	@command -v $(AS) >/dev/null || (echo "ERRO: nasm nao encontrado" && exit 1)
	@command -v grub-mkrescue >/dev/null || (echo "ERRO: grub-mkrescue nao encontrado" && exit 1)
	@command -v xorriso >/dev/null || (echo "ERRO: xorriso nao encontrado" && exit 1)

# Criação de pastas
dirs:
	@mkdir -p $(OBJDIR) $(ISODIR)/boot/grub

# --- ASM ---

$(OBJDIR)/multiboot.o: boot/multiboot.s | dirs
	$(AS) -f elf32 $< -o $@

$(OBJDIR)/boot.o: boot/boot.s | dirs
	$(AS) -f elf32 $< -o $@

$(OBJDIR)/isr_halt.o: boot/isr_halt.s | dirs
	$(AS) -f elf32 $< -o $@

$(OBJDIR)/interrupts.o: boot/interrupts.s | dirs
	$(AS) -f elf32 $< -o $@

# --- C ---

$(OBJDIR)/kernel.o: kernel/kernel.c include/console.h include/idt.h include/video.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/idt.o: kernel/idt.c include/idt.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/isr.o: kernel/isr.c include/isr.h include/console.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/irq.o: kernel/irq.c include/irq.h include/isr.h include/pic.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@
	
$(OBJDIR)/pic.o: kernel/pic.c include/pic.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/cmos.o: kernel/cmos.c include/cmos.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/memory.o: kernel/memory.c include/memory.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/sysconfig.o: kernel/sysconfig.c include/sysconfig.h include/memory.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/keyboard.o: drivers/keyboard.c include/keyboard.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/time.o: kernel/time.c include/time.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

# --- NOVOS DRIVERS DE VIDEO ---

$(OBJDIR)/video.o: drivers/video.c include/video.h include/multiboot.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/video_vesa.o: drivers/video_vesa.c include/video.h include/multiboot.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/console.o: drivers/console.c include/console.h include/font.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

# --- Link ---

$(KERNEL): $(OBJS) boot/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

# --- ISO ---

iso: check-tools $(KERNEL)
	@cp -f $(KERNEL) $(ISODIR)/boot/kernel.bin
	@cp -f boot/grub/grub.cfg $(ISODIR)/boot/grub/grub.cfg
	@echo "[ISO] Gerando $(ISO) ..."
	@grub-mkrescue -o $(ISO) $(ISODIR)
	@echo "OK: gerado $(ISO)"

run: iso
	qemu-system-i386 -m 512 -cdrom $(ISO)

clean:
	rm -rf $(BUILD)
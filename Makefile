# Tervia Cinser - i386
# Layout:
#   boot/ (boot.s, multiboot.s, linker.ld, grub/grub.cfg)
#   kernel/ (kernel.c, idt.c)
#   drivers/ (vga.c)
#   include/ (vga.h, idt.h)
# Output:
#   build/kernel.bin  (ELF32 multiboot; pode chamar .bin sem problemas)
#   build/Cinser.iso

NAME    := Cinser
BUILD   := build
OBJDIR  := $(BUILD)/obj
ISODIR  := $(BUILD)/isodir

KERNEL  := $(BUILD)/kernel.bin
ISO     := $(BUILD)/$(NAME).iso

CC      := gcc
LD      := ld
AS      := nasm

CFLAGS  := -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -Wall -Wextra -Iinclude \
           -fno-asynchronous-unwind-tables -fno-omit-frame-pointer
LDFLAGS := -m elf_i386 -T boot/linker.ld

OBJS := \
  $(OBJDIR)/multiboot.o \
  $(OBJDIR)/boot.o \
  $(OBJDIR)/isr_halt.o \
  $(OBJDIR)/kernel.o \
  $(OBJDIR)/idt.o \
  $(OBJDIR)/vga.o

.PHONY: all iso run clean dirs check-tools

all: iso

check-tools:
	@command -v $(AS) >/dev/null || (echo "ERRO: nasm nao encontrado" && exit 1)
	@command -v grub-mkrescue >/dev/null || (echo "ERRO: grub-mkrescue nao encontrado" && exit 1)
	@command -v xorriso >/dev/null || (echo "ERRO: xorriso nao encontrado" && exit 1)

# Pastas

dirs:
	@mkdir -p $(OBJDIR) $(ISODIR)/boot/grub

# ASM

$(OBJDIR)/multiboot.o: boot/multiboot.s | dirs
	$(AS) -f elf32 $< -o $@

$(OBJDIR)/boot.o: boot/boot.s | dirs
	$(AS) -f elf32 $< -o $@

$(OBJDIR)/isr_halt.o: boot/isr_halt.s | dirs
	$(AS) -f elf32 $< -o $@

# C

$(OBJDIR)/kernel.o: kernel/kernel.c include/vga.h include/idt.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/idt.o: kernel/idt.c include/idt.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/vga.o: drivers/vga.c include/vga.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

# Link

$(KERNEL): $(OBJS) boot/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

# ISO

iso: check-tools $(KERNEL)
	@cp -f $(KERNEL) $(ISODIR)/boot/kernel.bin
	@cp -f boot/grub/grub.cfg $(ISODIR)/boot/grub/grub.cfg
	@echo "[ISO] Gerando $(ISO) ..."
	@grub-mkrescue -o $(ISO) $(ISODIR)
	@echo "OK: gerado $(ISO)"

run: iso
	qemu-system-i386 -m 256 -cdrom $(ISO)

clean:
	rm -rf $(BUILD)

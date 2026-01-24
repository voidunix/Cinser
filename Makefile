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

# Fedora costuma usar grub2-mkrescue ao invés de grub-mkrescue.
GRUB_MKRESCUE := $(shell command -v grub-mkrescue 2>/dev/null || command -v grub2-mkrescue 2>/dev/null)

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
  $(OBJDIR)/delay.o \
  $(OBJDIR)/math.o \
  $(OBJDIR)/keyboard.o \
  $(OBJDIR)/mouse.o \
  $(OBJDIR)/video.o \
  $(OBJDIR)/video_vesa.o \
  $(OBJDIR)/console.o \
  $(OBJDIR)/desktop.o \
  $(OBJDIR)/window.o \
  $(OBJDIR)/shell.o \
  $(OBJDIR)/shice.o \
  $(OBJDIR)/splash.o \
  $(OBJDIR)/shice_sinfetch.o \
  $(OBJDIR)/shice_help.o

.PHONY: all iso run clean dirs check-tools

all: iso

check-tools:
	@command -v $(AS) >/dev/null || (echo "ERRO: nasm nao encontrado" && exit 1)
	@[ -n "$(GRUB_MKRESCUE)" ] || (echo "ERRO: grub-mkrescue/grub2-mkrescue nao encontrado" && exit 1)
	@command -v xorriso >/dev/null || (echo "ERRO: xorriso nao encontrado" && exit 1)
	@command -v grub-mkstandalone >/dev/null || true

# Criação de pastas
dirs:
	@mkdir -p $(OBJDIR) $(ISODIR)/boot/grub $(ISODIR)/EFI/BOOT

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

$(OBJDIR)/mouse.o: drivers/mouse.c include/mouse.h include/irq.h include/pic.h include/io.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/time.o: kernel/time.c include/time.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/delay.o: kernel/delay.c include/delay.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/math.o: kernel/math.c include/math.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@


# --- NOVOS DRIVERS DE VIDEO ---

$(OBJDIR)/video.o: drivers/video.c include/video.h include/multiboot.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/video_vesa.o: drivers/video_vesa.c include/video.h include/multiboot.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/console.o: drivers/console.c include/console.h include/font.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/desktop.o: kernel/desktop.c include/desktop.h include/window.h include/video.h include/font.h include/programs/shell.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/window.o: kernel/window.c include/window.h include/video.h include/font.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/shell.o: programs/shell.c include/programs/shell.h include/window.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/shice.o: programs/shice.c include/programs/shice.h include/console.h include/keyboard.h include/shice/shice_help.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/shice_help.o: shice/shice_help.c include/shice/shice_help.h include/console.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/shice_sinfetch.o: shice/shice_sinfetch.c include/shice/shice_sinfetch.h include/console.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/splash.o: kernel/splash.c include/splash.h | dirs
	$(CC) $(CFLAGS) -c $< -o $@

# --- Link ---

$(KERNEL): $(OBJS) boot/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

# --- ISO ---

iso: check-tools $(KERNEL)
	@cp -f $(KERNEL) $(ISODIR)/boot/kernel.bin
	@cp -f boot/grub/grub.cfg $(ISODIR)/boot/grub/grub.cfg
	@cp -f boot/grub/uefi-early.cfg $(ISODIR)/boot/grub/uefi-early.cfg
	@# -----------------------------------------------------------------------
	@# UEFI (x86_64) opcional: gera BOOTX64.EFI via grub-mkstandalone.
	@# Isso permite boot em máquinas UEFI modernas, mantendo o kernel i386.
	@# Requisito típico no Fedora/Debian: pacote grub2-efi-x64 ou grub-efi-amd64.
	@# Se não estiver instalado, o ISO continua bootável via BIOS/CSM.
	@# -----------------------------------------------------------------------
	@if command -v grub-mkstandalone >/dev/null 2>&1 && [ -d /usr/lib/grub/x86_64-efi ]; then \
		echo "[UEFI] Gerando BOOTX64.EFI (GRUB standalone)..."; \
		grub-mkstandalone -O x86_64-efi -d /usr/lib/grub/x86_64-efi -o $(ISODIR)/EFI/BOOT/BOOTX64.EFI \
			"boot/grub/grub.cfg=boot/grub/uefi-early.cfg"; \
	else \
		echo "[UEFI] BOOTX64.EFI nao gerado (grub-mkstandalone ou /usr/lib/grub/x86_64-efi ausente)."; \
		echo "      Dica: instale grub2-efi-x64 (Fedora) / grub-efi-amd64-bin (Debian/Ubuntu)."; \
	fi
	@echo "[ISO] Gerando $(ISO) ..."
	@$(GRUB_MKRESCUE) -o $(ISO) $(ISODIR)
	@echo "OK: gerado $(ISO)"

run: iso
	qemu-system-i386 -m 512 -cdrom $(ISO)

clean:
	rm -rf $(BUILD)
; ---------------------------------------------------------------------------
; Tervia Cinser OS - Sistema Operacional
; Desenvolvido por: Tervia Corporation (2026)
; 
; Licença: GNU GPLv3
; Este arquivo faz parte do projeto Tervia Cinser. 
; Você tem a liberdade de estudar, modificar e distribuir este código
; desde que mantenha esta licença original.
; ---------------------------------------------------------------------------
; Descrição: Rotinas de bootloader e inicialização de registradores.
; ---------------------------------------------------------------------------

SECTION .multiboot
align 4

%define MULTIBOOT_MAGIC     0x1BADB002
%define MULTIBOOT_PAGE_ALIGN (1 << 0)
%define MULTIBOOT_MEMORY_INFO (1 << 1)
%define MULTIBOOT_VIDEO_MODE  (1 << 2) ; <-- Pedido de vídeo

%define MULTIBOOT_FLAGS     (MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_VIDEO_MODE)
%define MULTIBOOT_CHECKSUM  -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

dd MULTIBOOT_MAGIC
dd MULTIBOOT_FLAGS
dd MULTIBOOT_CHECKSUM

; Campos de endereço (não usados se bit 16 estiver desligado, mas mantemos o espaço)
dd 0, 0, 0, 0, 0

; --- Configuração de Vídeo ---
; 0 = Gráfico Linear, 1 = Texto
dd 0    
; Largura, Altura, Profundidade (Bits por Pixel)
dd 1024, 768, 32

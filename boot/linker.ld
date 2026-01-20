ENTRY(_start)

SECTIONS
{
  . = 1M;

  .multiboot : {
    KEEP(*(.multiboot))
  }

  .text : {
    *(.text*)
  }

  .rodata : {
    *(.rodata*)
  }

  .data : {
    *(.data*)
  }

  .bss : {
    *(COMMON)
    *(.bss*)
  }

  /* Fim do kernel (usado pelo memory manager) */
  PROVIDE(_kernel_end = .);
}

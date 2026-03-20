ENTRY(_start)

SECTIONS
{
  . = 1M;

  _kernel_start = .;

  .multiboot2 : ALIGN(8) { *(.multiboot2) }

  .text : ALIGN(16) { *(.text*) }
  .rodata : ALIGN(16) { *(.rodata*) }
  .data : ALIGN(16) { *(.data*) }
  .bss : ALIGN(16) {
    __bss_start = .;
    *(COMMON) *(.bss*)
    __bss_end = .;
  }

  _kernel_end = ALIGN(4096);
}
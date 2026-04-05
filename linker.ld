ENTRY(_start)

KERNEL_VMA = 0xFFFFFFFF80000000;
KERNEL_LMA = 0x100000;

SECTIONS
{
  . = KERNEL_VMA + KERNEL_LMA;

  _kernel_start = .;

  .multiboot2 ALIGN(8) : AT(ADDR(.multiboot2) - KERNEL_VMA)
  {
    *(.multiboot2)
  }

  .text ALIGN(16) : AT(ADDR(.text) - KERNEL_VMA)
  {
    *(.text*)
  }

  .rodata ALIGN(16) : AT(ADDR(.rodata) - KERNEL_VMA)
  {
    *(.rodata*)
  }

  .data ALIGN(16) : AT(ADDR(.data) - KERNEL_VMA)
  {
    *(.data*)
  }

  .bss ALIGN(16) : AT(ADDR(.bss) - KERNEL_VMA)
  {
    __bss_start = .;
    *(COMMON) *(.bss*)
    __bss_end = .;
  }

  _kernel_end = ALIGN(4096);

  __bss_start_phys = LOADADDR(.bss);
  __bss_end_phys   = LOADADDR(.bss) + SIZEOF(.bss);

  stack_top_phys = LOADADDR(.bss) + (stack_top - ADDR(.bss));
  stack_bottom_phys = LOADADDR(.bss) + (stack_bottom - ADDR(.bss));

  pml4_phys = LOADADDR(.data) + (pml4 - ADDR(.data));
  pdpt_phys = LOADADDR(.data) + (pdpt - ADDR(.data));
  pd_phys   = LOADADDR(.data) + (pd   - ADDR(.data));
  pd1_phys  = LOADADDR(.data) + (pd1  - ADDR(.data));
  pd2_phys  = LOADADDR(.data) + (pd2  - ADDR(.data));
  pd3_phys  = LOADADDR(.data) + (pd3  - ADDR(.data));
  pdpt_direct_phys = LOADADDR(.data) + (pdpt_direct - ADDR(.data));

  gdt64_phys = LOADADDR(.data) + (gdt64 - ADDR(.data));
  gdt64_desc_phys = LOADADDR(.data) + (gdt64_desc - ADDR(.data));

  start64_phys = LOADADDR(.text) + (start64 - ADDR(.text));
}
/* Multiboot2 + 32-bit entry -> enable long mode -> jump to 64-bit -> call kernel_main */

.set MB2_MAGIC,     0xE85250D6
.set MB2_ARCH,      0
.set MB2_HDR_LEN,   (mb2_end - mb2_start)
.set MB2_CHECKSUM,  -(MB2_MAGIC + MB2_ARCH + MB2_HDR_LEN)

.section .multiboot2
.align 8
mb2_start:
  .long MB2_MAGIC
  .long MB2_ARCH
  .long MB2_HDR_LEN
  .long MB2_CHECKSUM

  /* end tag */
  .short 0
  .short 0
  .long 8
mb2_end:

/* stack can be in .bss (zeroed) */
.section .bss
.align 16
stack_bottom:
  .skip 16384
stack_top:

/* tables contain non-zero initial values -> keep them in .data */
.section .data
.align 4096
pml4:
  .quad 0
  .skip 4096-8

.align 4096
pdpt:
  .quad 0
  .skip 4096-8

.align 4096
pd:
  /* 512 entries, each maps 2MiB: addr | present|rw|ps(2MiB) = 0x83 */
  .set i, 0
  .rept 512
    .quad (i * 0x200000) + 0x83
    .set i, i+1
  .endr

/* GDT (minimal) */
.align 16
gdt64:
  .quad 0x0000000000000000
  .quad 0x00AF9A000000FFFF
  .quad 0x00AF92000000FFFF
gdt64_end:

gdt64_desc:
  .word gdt64_end - gdt64 - 1
  .long gdt64

.section .text
.global _start
.type _start, @function

.code32
_start:
  cli
  mov $stack_top, %esp

  lgdt gdt64_desc

  /* pml4[0] -> pdpt */
  movl $pdpt, %eax
  orl  $0x003, %eax
  movl %eax, pml4

  /* pdpt[0] -> pd */
  movl $pd, %eax
  orl  $0x003, %eax
  movl %eax, pdpt

  /* CR4.PAE=1 */
  mov %cr4, %eax
  or  $0x20, %eax
  mov %eax, %cr4

  /* CR3 = pml4 */
  movl $pml4, %eax
  mov %eax, %cr3

  /* EFER.LME=1 */
  mov $0xC0000080, %ecx
  rdmsr
  or  $0x00000100, %eax
  wrmsr

  /* CR0.PG=1 */
  mov %cr0, %eax
  or  $0x80000000, %eax
  mov %eax, %cr0

  ljmp $0x08, $start64

.code64
start64:
  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %ss
  mov %ax, %fs
  mov %ax, %gs

  mov $stack_top, %rsp
  and $-16, %rsp

  call kernel_main

.hang:
  hlt
  jmp .hang

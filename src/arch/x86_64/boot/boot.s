/* Multiboot2 + 32-bit entry -> enable long mode -> jump to 64-bit -> call kernel_main */

.set MB2_MAGIC,     0xE85250D6
.set MB2_ARCH,      0
.set MB2_HDR_LEN,   (mb2_end - mb2_start)
.set MB2_CHECKSUM,  -(MB2_MAGIC + MB2_ARCH + MB2_HDR_LEN)

.section .multiboot2, "a", @progbits
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

.global stack_bottom
.global stack_top

stack_bottom:
  .skip 16384
stack_top:

/* tables contain non-zero initial values -> keep them in .data */
.section .data
.align 4096

.global pml4
pml4:
  .quad 0
  .skip 4096-8

.align 4096
.global pdpt
pdpt:
  .quad 0
  .skip 4096-8

.align 4096
.global pd
pd:
  /* 512 entries, each maps 2MiB: addr | present|rw|ps(2MiB) = 0x83 */
  .set i, 0
  .rept 512
    .quad (i * 0x200000) + 0x83
    .set i, i+1
  .endr

/* GDT (minimal) */
.global gdt64
.global gdt64_end
.global gdt64_desc

gdt64:
  .quad 0
  .quad 0x00AF9A000000FFFF
  .quad 0x00AF92000000FFFF
gdt64_end:

gdt64_desc:
  .word gdt64_end - gdt64 - 1
  .long gdt64_phys

.section .note.GNU-stack,"",%progbits

.section .text
.global _start
.type _start, @function

.code32
_start:
  cli
  cld
  /* Preserve Multiboot registers before clobbering general-purpose regs. */
  mov %eax, %esi   /* magic -> esi */
  mov %ebx, %edx   /* info  -> edx (edx is not clobbered by BSS init) */
  /* ES = DS (GRUB may leave ES undefined) */
  push %ds
  pop  %es
  /* Zero BSS so C globals are 0 and we do not rely on loader */
  mov $__bss_end_phys, %ecx
  mov $__bss_start_phys, %edi
  sub  %edi, %ecx
  xor  %eax, %eax
  rep  stosb
  mov  $stack_top_phys, %esp
  /* Save preserved Multiboot magic/info for 64-bit entry. */
  push %edx   /* info  -> [rsp+4] after next push */
  push %esi   /* magic -> [rsp]                   */

  lgdt gdt64_desc_phys

  /* pml4[0] -> pdpt */
  movl $pdpt_phys, %eax
  orl  $0x003, %eax
  movl %eax, pml4_phys

  /* pdpt[0] -> pd */
  movl $pd_phys, %eax
  orl  $0x003, %eax
  movl %eax, pdpt_phys

  /* CR4.PAE=1 */
  mov %cr4, %eax
  or  $0x20, %eax
  mov %eax, %cr4

  /* CR3 = pml4 */
  movl $pml4_phys, %eax
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

  jmp long_mode_entry

.code32
long_mode_entry:
  pushl $0x08
  pushl $start64_phys
  lret

.global start64

.code64
start64:
  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %ss
  mov %ax, %fs
  mov %ax, %gs

  /* 32-bit entry pushed EAX/EBX as 4-byte values: [rsp]=magic, [rsp+4]=info. */
  mov (%rsp), %edi
  mov 4(%rsp), %esi
  add $8, %rsp
  mov $stack_top, %rsp
  and $-16, %rsp

  call kmain

.hang:
  hlt
  jmp .hang

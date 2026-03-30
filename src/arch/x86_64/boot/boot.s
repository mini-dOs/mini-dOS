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
  .fill 4096, 1, 0

.align 4096
.global pdpt
pdpt:
  .fill 4096, 1, 0

.align 4096
.global pd
pd:
  .fill 4096, 1, 0

.align 4096
.global pd1
pd1:
  .fill 4096, 1, 0

.align 4096
.global pd2
pd2:
  .fill 4096, 1, 0

.align 4096
.global pd3
pd3:
  .fill 4096, 1, 0

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

  /* Preserve Multiboot registers */
  mov %eax, %esi   /* magic -> esi */
  mov %ebx, %edx   /* info  -> edx (edx is not clobbered by BSS init) */

  /* ES = DS */
  push %ds
  pop  %es

  /* ---------------------------
   * BSS zeroing
   * --------------------------- */
  mov $__bss_start_phys, %edi
  mov $__bss_end_phys, %ecx
  sub  %edi, %ecx
  xor  %eax, %eax
  rep  stosb

  /* ---------------------------
   * stack setup
   * --------------------------- */
  mov  $stack_top_phys, %esp

  /* Save multiboot params for 64-bit */
  push %edx   /* info */
  push %esi   /* magic */

  /* ---------------------------
   * Load GDT
   * --------------------------- */
  lgdt gdt64_desc_phys

  /* ---------------------------
   * Clear page tables
   * --------------------------- */
  /* PML4 */
  mov $pml4_phys, %edi
  mov $1024, %ecx
  xor %eax, %eax
  rep stosl

  /* PDPT */
  mov $pdpt_phys, %edi
  mov $1024, %ecx
  xor %eax, %eax
  rep stosl

  /* PD */
  mov $pd_phys, %edi
  mov $1024, %ecx
  xor %eax, %eax
  rep stosl

  /* PD1 */
  mov $pd1_phys, %edi
  mov $1024, %ecx
  xor %eax, %eax
  rep stosl

  /* PD2 */
  mov $pd2_phys, %edi
  mov $1024, %ecx
  xor %eax, %eax
  rep stosl

  /* PD3 */
  mov $pd3_phys, %edi
  mov $1024, %ecx
  xor %eax, %eax
  rep stosl

  /* ---------------------------
   * Build paging structures
   * --------------------------- */
  /* PML4[0] → PDPT (identity) */
  mov $pdpt_phys, %eax
  or  $0x003, %eax
  mov %eax, (pml4_phys)
  movl $0, (pml4_phys + 4)

  /* PML4[511] → PDPT (higher-half) */
  mov $pdpt_phys, %eax
  or  $0x003, %eax
  mov %eax, (pml4_phys + 4088)
  movl $0, (pml4_phys + 4092)

  /* PDPT[0] → PD (identity) */
  mov $pd_phys, %eax
  or  $0x003, %eax
  mov %eax, (pdpt_phys)
  movl $0, (pdpt_phys + 4)

  /* PDPT[1] → PD1 (identity) */
  mov $pd1_phys, %eax
  or  $0x003, %eax
  mov %eax, (pdpt_phys + 8)
  movl $0, (pdpt_phys + 12)

  /* PDPT[2] → PD2 (identity) */
  mov $pd2_phys, %eax
  or  $0x003, %eax
  mov %eax, (pdpt_phys + 16)
  movl $0, (pdpt_phys + 20)

  /* PDPT[3] → PD3 (identity) */
  mov $pd3_phys, %eax
  or  $0x003, %eax
  mov %eax, (pdpt_phys + 24)
  movl $0, (pdpt_phys + 28)

  /* PDPT[510] → PD (higher-half window for ffffffff80000000) */
  mov $pd_phys, %eax
  or  $0x003, %eax
  mov %eax, (pdpt_phys + 4080)
  movl $0, (pdpt_phys + 4084)
  
  /* ---------------------------
   * Fill PDs with 2MB pages
   * identity mapping: 0 ~ 4GB
   * --------------------------- */
  mov $pd_phys, %edi
  mov $512, %ecx
  xor %eax, %eax        /* physical addr = 0 */

.map_pd0:
  mov %eax, %ebx
  or  $0x083, %ebx      /* present | rw | huge page */
  mov %ebx, (%edi)
  movl $0, 4(%edi)

  add $0x200000, %eax   /* +2MB */
  add $8, %edi
  loop .map_pd0

  mov $pd1_phys, %edi
  mov $512, %ecx
  mov $0x40000000, %eax /* physical addr = 1GB */

.map_pd1:
  mov %eax, %ebx
  or  $0x083, %ebx
  mov %ebx, (%edi)
  movl $0, 4(%edi)

  add $0x200000, %eax
  add $8, %edi
  loop .map_pd1

  mov $pd2_phys, %edi
  mov $512, %ecx
  mov $0x80000000, %eax /* physical addr = 2GB */

.map_pd2:
  mov %eax, %ebx
  or  $0x083, %ebx
  mov %ebx, (%edi)
  movl $0, 4(%edi)

  add $0x200000, %eax
  add $8, %edi
  loop .map_pd2

  mov $pd3_phys, %edi
  mov $512, %ecx
  mov $0xC0000000, %eax /* physical addr = 3GB */

.map_pd3:
  mov %eax, %ebx
  or  $0x083, %ebx
  mov %ebx, (%edi)
  movl $0, 4(%edi)

  add $0x200000, %eax
  add $8, %edi
  loop .map_pd3

  /* ---------------------------
   * Enable long mode
   * --------------------------- */
  /* CR4.PAE = 1 */
  mov %cr4, %eax
  or  $0x20, %eax
  mov %eax, %cr4

  /* CR3 = pml4 */
  mov $pml4_phys, %eax
  mov %eax, %cr3

  /* EFER.LME = 1 */
  mov $0xC0000080, %ecx
  rdmsr
  or  $0x00000100, %eax
  wrmsr

  /* CR0.PG = 1 */
  mov %cr0, %eax
  or  $0x80000000, %eax
  mov %eax, %cr0

  /* ---------------------------
   * Far jump to 64-bit
   * --------------------------- */
  ljmp $0x08, $start64_phys

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
  mov $stack_top_phys, %rsp
  and $-16, %rsp

  movabs $higher_half_entry, %rax
  jmp *%rax

higher_half_entry:
  call kmain

.hang:
  hlt
  jmp .hang

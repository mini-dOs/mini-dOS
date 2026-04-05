# mini-dOs

x86_64 아키텍처를 대상으로 하는 **미니 운영체제 커널 프로젝트**입니다.
GRUB + Multiboot2를 사용해 부팅하며, Long Mode로 진입한 뒤 C 커널로 제어를 전달합니다.

현재 목표:

* GRUB 기반 부팅
* x86_64 Long Mode 진입
* GDT 초기화
* 기본 커널 실행 환경 구축

---

# 📂 프로젝트 구조 (Project Structure)

```
.
├── build
├── compile_flags.txt
├── isodir
│   └── boot
│       └── grub
│           └── grub.cfg
├── linker.ld
├── Makefile
├── README.md
└── src
    ├── arch
    │   └── x86_64
    │       ├── boot
    │       │   └── boot.s
    │       ├── drivers
    │       │   ├── i8042.c
    │       │   ├── inb.S
    │       │   └── outb.S
    │       ├── gdt
    │       │   ├── gdt.c
    │       │   ├── lgdt_asm.S
    │       │   └── ltr_asm.S
    │       └── interrupt
    │           ├── idt.c
    │           ├── idt_load.S
    │           ├── isr_stub.S
    │           ├── isr_table.c
    │           └── pic.c
    ├── drivers
    │   ├── keyboard.c
    │   └── serial.c
    ├── include
    │   ├── mm
    │   │   ├── paging.h
    │   │   ├── pmm.h
    │   │   └── vmm.h
    │   ├── config.h
    │   ├── cpu.h
    │   ├── early_alloc.h
    │   ├── exception.h
    │   ├── gdt.h
    │   ├── i8042.h
    │   ├── idt.h
    │   ├── interrupt.h
    │   ├── interrupt_init.h
    │   ├── irq.h
    │   ├── kernel_base.h
    │   ├── kernel_info.h
    │   ├── keyboard.h
    │   ├── multiboot.h
    │   ├── multiboot2.h
    │   ├── pic.h
    │   └── pit.h
    ├── kernel
    │   └── kmain.c
    ├── lib
    │   └── string.c
    └── mm
        ├── paging.c
        ├── pmm.c
        └── vmm.c
```

---

# 주요 디렉터리 설명

| 경로                              | 역할                                            |
| ------------------------------- | --------------------------------------------- |
| **src/**                        | 커널의 모든 소스 코드가 위치하는 루트 디렉터리                    |
| **src/arch/**                   | CPU 아키텍처 의존 코드                                |
| **src/arch/x86_64/**            | x86_64 전용 구현                                  |
| **src/arch/x86_64/boot/**       | 부트 엔트리 코드 (Multiboot2 → Long Mode 전환)         |
| **src/arch/x86_64/drivers/**    | 아키텍처 의존 드라이버 (i8042, I/O 포트)                  |
| **src/arch/x86_64/gdt/**        | GDT 및 TSS 초기화 코드                              |
| **src/arch/x86_64/interrupt/**  | IDT, PIC, ISR 스텁 및 인터럽트 핸들러                   |
| **src/kernel/**                 | 아키텍처와 독립적인 커널 핵심 코드                           |
| **src/drivers/**                | 장치 드라이버 코드 (키보드, 시리얼)                         |
| **src/lib/**                    | 커널 라이브러리 (string.c 등 freestanding 유틸리티)        |
| **src/mm/**                     | 메모리 관리 (PMM, Paging, VMM)                     |
| **src/include/**                | 커널 공용 헤더 파일                                   |
| **src/include/mm/**             | 메모리 관리 관련 헤더 (paging, pmm, vmm)                |

---

# 빌드 시스템

| 파일                    | 역할                   |
| --------------------- | -------------------- |
| **Makefile**          | 커널 빌드 자동화            |
| **linker.ld**         | 커널 메모리 레이아웃 정의       |
| **compile_flags.txt** | clangd / LSP용 컴파일 옵션 |

빌드 결과물은 다음 위치에 생성됩니다.

```
build/kernel.elf
build/myos.iso
```

---

# 부팅 과정 (Boot Flow)

커널 부팅 과정은 다음과 같습니다.

```
GRUB
  ↓
Multiboot2 Loader
  ↓
boot.s (32-bit)
  ↓
페이지 테이블 구축 (Identity-map + Higher-half + Direct-map)
  ↓
PAE / EFER.LME / Paging 활성화 → Long Mode 진입
  ↓
Higher-half 가상 주소로 전환
  ↓
kmain() 실행
```

---

# 실행 방법

## 빌드

```
make
```

## ISO 생성

```
make iso
```

## QEMU 실행

```
make run
```

---

# 개발 환경

권장 환경

```
Ubuntu 22.04
x86_64-elf-gcc
QEMU
GRUB
clangd
```

---

# 현재 구현 상태

* [x] Multiboot2 부팅
* [x] Long Mode 전환
* [x] GDT / TSS 초기화
* [x] Higher-half 커널 (가상 주소 `0xFFFFFFFF80000000`)
* [x] 페이징 (4KB/2MB 페이지, PML4 기반)
* [x] 물리 메모리 관리 (Buddy PMM, 4KB–4MB)
* [x] Direct-map (`0xFFFF888000000000`)
* [x] IDT 구현
* [x] PIC (8259A) 초기화
* [x] 인터럽트 / 예외 처리 (ISR 스텁 + C 핸들러)
* [x] 시리얼 콘솔 (COM1)
* [x] PS/2 키보드 드라이버
* [ ] 가상 메모리 관리 (VMM) 완성
* [ ] 프로세스 / 스케줄러
* [ ] 시스템 콜
* [ ] 파일 시스템
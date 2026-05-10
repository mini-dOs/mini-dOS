# mini-dOs

x86_64 아키텍처를 대상으로 하는 **미니 운영체제 커널 프로젝트**입니다.
GRUB + Multiboot2를 사용해 부팅하며, Long Mode로 진입한 뒤 C 커널로 제어를 전달합니다.

현재 목표:

* GRUB 기반 부팅
* x86_64 Long Mode 진입
* GDT / IDT / 페이징 / PMM / VMM / Slab / vmalloc
* DOOM 포팅 (진행 중)

---

# 📂 프로젝트 구조 (Project Structure)

리눅스 트리를 본떠, *서브시스템별로 top-level에* 배치합니다 (`src/` wrapper 없음).

```
.
├── arch/
│   └── x86_64/
│       ├── boot/                  # boot.s (Multiboot2 → Long Mode)
│       ├── gdt/                   # GDT / TSS 초기화
│       └── interrupt/             # IDT, ISR 스텁, 8259 PIC
├── drivers/                       # 디바이스 드라이버 (카테고리별)
│   ├── input/                     # i8042 (PS/2 컨트롤러), 키보드
│   ├── tty/                       # 시리얼 (UART 16550)
│   └── video/                     # 프레임버퍼, 폰트
├── init/
│   └── kmain.c                    # 부팅 진입점
├── kernel/                        # 아키텍처 무관 커널 핵심
│                                  #   multiboot, irq, exception, pit, early_alloc, ...
├── mm/                            # 메모리 관리 (paging, pmm, vmm, slab, vmalloc)
├── libc/                          # POSIX 스타일 shim 레이어 (DOOM용)
│   ├── include/                   # stdio.h, stdlib.h, string.h, ...
│   └── src/                       # 구현
├── doom/                          # vendored ozkl/doomgeneric (포팅 진행 중)
├── include/                       # 커널 공용 헤더 (네임스페이스별)
│   ├── asm/                       # x86 specific (cpu, gdt, idt, irq, pic, interrupt)
│   ├── kernel/                    # arch-neutral (multiboot, kernel_base, pit, ...)
│   ├── drivers/                   # 디바이스 헤더
│   └── mm/                        # 메모리 헤더
├── isodir/
│   └── boot/grub/grub.cfg
├── linker.ld
└── Makefile
```

---

# 헤더 네임스페이스 규칙

include 경로만 봐도 헤더의 *정체*가 보이게 설계:

| Include 형태 | 의미 | 예 |
| --- | --- | --- |
| `<asm/foo.h>` | x86 전용. 다른 arch 포팅 시 갈아엎음 | `<asm/cpu.h>`, `<asm/idt.h>` |
| `<kernel/foo.h>` | arch-neutral 커널 서비스 API | `<kernel/multiboot.h>`, `<kernel/pit.h>` |
| `<drivers/foo.h>` | 일반 디바이스 드라이버 | `<drivers/framebuffer.h>` |
| `<mm/foo.h>` | 메모리 관리 | `<mm/vmm.h>` |
| `<stdio.h>` 등 | libc shim (DOOM 및 일부 커널 — `<string.h>`) | `<string.h>`, `<stdio.h>` |

Makefile은 빌드 대상별로 include path를 다르게 줘서 **DOOM이 커널 internal을 import 못하게 강제**합니다 — DOOM ↔ 커널 경계는 `doom/doomgeneric.h`의 4개 콜백 (`DG_DrawFrame`, `DG_GetKey`, `DG_GetTicksMs`, `DG_SleepMs`)만 통과하는 구조.

---

# 빌드 시스템

| 파일 | 역할 |
| --- | --- |
| **Makefile** | 빌드 자동화. `arch/`, `drivers/`, `init/`, `kernel/`, `mm/`, `libc/src/`, `doom/` 하위의 `.c`/`.S`/`.s` 자동 수집 |
| **linker.ld** | 커널 메모리 레이아웃 정의 |

빌드 결과물:

```
build/kernel.elf
build/myos.iso
```

---

# 부팅 과정 (Boot Flow)

```
GRUB
  ↓
Multiboot2 Loader
  ↓
arch/x86_64/boot/boot.s (32-bit)
  ↓
페이지 테이블 구축 (Identity-map + Higher-half + Direct-map)
  ↓
PAE / EFER.LME / Paging 활성화 → Long Mode 진입
  ↓
Higher-half 가상 주소로 전환
  ↓
init/kmain.c — kmain() 실행
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

## 디버그 (gdb stub :1234)

```
make run-debug
```

---

# 개발 환경

권장 환경:

```
Ubuntu 22.04
x86_64-elf-gcc      (cross-compiler at /opt/cross/bin/)
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
* [x] 페이징 (4KB / 2MB 페이지, PML4 기반)
* [x] 물리 메모리 관리 (Buddy PMM, 4KB–4MB)
* [x] Direct-map (`0xFFFF888000000000`)
* [x] IDT 구현
* [x] PIC (8259A) 초기화
* [x] 인터럽트 / 예외 처리 (ISR 스텁 + C 핸들러)
* [x] 시리얼 콘솔 (COM1)
* [x] PS/2 키보드 드라이버
* [x] 가상 메모리 관리 (VMM, ownership-aware)
* [x] Slab 할당자 (8B–2KB, 9개 캐시)
* [x] vmalloc (linked-list, first-fit)
* [x] 트리 구조 리팩토링 (Linux 풍 서브시스템 레이아웃)
* [ ] DOOM 포팅 (libc shim 구현 + Multiboot2 module 로딩 + 8bpp→32bpp blit)
* [ ] 프로세스 / 스케줄러
* [ ] 시스템 콜
* [ ] 파일 시스템

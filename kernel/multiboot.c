#include <drivers/serial.h>
#include <kernel/early_alloc.h>
#include <kernel/kernel_base.h>
#include <kernel/multiboot.h>
#include <kernel/multiboot2.h>
#include <stdint.h>

extern char _kernel_end[];

memory_region_t *usable_regions;
fb_info_t fb_info;
uint32_t usable_region_count;
boot_module_t boot_modules[MAX_BOOT_MODULES];
uint32_t boot_module_count;

// Utility
static uint64_t align_up(uint64_t addr) {
    return (addr + 0xFFF) & ~0xFFF;
}

static uint64_t align_down(uint64_t addr) {
    return addr & ~0xFFF;
}

// ###################################################################################################
// Dump current usable memory regions (for debugging)
static void dump_usable_regions(const char *label) {
    uint64_t total = 0;

    serial_write("[MMAP] ");
    serial_write(label);
    serial_write(", count=");
    serial_write_dec(usable_region_count);
    serial_write("\n");

    for (uint32_t i = 0; i < usable_region_count; i++) {
        uint64_t size = usable_regions[i].end - usable_regions[i].start;
        serial_write("[MMAP] region[");
        serial_write_dec(i);
        serial_write("] start=");
        serial_write_hex64(usable_regions[i].start);
        serial_write(" end=");
        serial_write_hex64(usable_regions[i].end);
        serial_write(" size=");
        serial_write_size(size);
        serial_write("\n");

        total += size;
    }

    serial_write("[MMAP] total usable = ");
    serial_write_size(total);
    serial_write("\n");
}
// ###################################################################################################

// ===== AI-GENERATED (Claude) BEGIN =====
// Copy the basename of a module cmdline into dst[cap] (NUL-terminated).
// 예: "/boot/doom1.wad" -> "doom1.wad". cmdline이 비어 있으면 dst는 빈 문자열.
static void copy_module_name(char *dst, uint32_t cap, const char *cmdline) {
    const char *base = cmdline;
    for (const char *p = cmdline; *p; p++) {
        if (*p == '/' || *p == '\\')
            base = p + 1;
    }

    uint32_t i = 0;
    while (base[i] && i + 1 < cap) {
        dst[i] = base[i];
        i++;
    }
    dst[i] = '\0';
}

// Record one GRUB boot module (skip if table full)
static void parse_module(struct multiboot_tag_module *mod_tag) {
    if (boot_module_count >= MAX_BOOT_MODULES)
        return;

    boot_module_t *m = &boot_modules[boot_module_count++];
    m->start = mod_tag->mod_start;
    m->end   = mod_tag->mod_end;
    copy_module_name(m->name, sizeof(m->name), mod_tag->cmdline);

    serial_write("[MODULE] '");
    serial_write(m->name);
    serial_write("' start=");
    serial_write_hex64(m->start);
    serial_write(" end=");
    serial_write_hex64(m->end);
    serial_write(" size=");
    serial_write_size(m->end - m->start);
    serial_write("\n");
}
// ===== AI-GENERATED (Claude) END ======

// Count MULTIBOOT_MEMORY_AVAILABLE entries
static uint32_t count_usable_regions(struct multiboot_tag_mmap *mmap_tag) {
    uint32_t count = 0;
    struct multiboot_mmap_entry *entry;

    for (entry = mmap_tag->entries;
         (uint8_t *)entry < (uint8_t *)mmap_tag + mmap_tag->size;
         entry = (struct multiboot_mmap_entry *)
                 ((uint8_t *)entry + mmap_tag->entry_size)) {

        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            count++;
        }
    }

    return count;
}

// Extract usable regions from multiboot mmap
static void parse_mmap(struct multiboot_tag_mmap *mmap_tag) {
    struct multiboot_mmap_entry *entry;

    usable_region_count = count_usable_regions(mmap_tag);

    // Allocate region array using early allocator
    usable_regions = early_alloc(
        usable_region_count * sizeof(memory_region_t),
        8
    );

    uint32_t idx = 0;

    for (entry = mmap_tag->entries;
         (uint8_t *)entry < (uint8_t *)mmap_tag + mmap_tag->size;
         entry = (struct multiboot_mmap_entry *)
                 ((uint8_t *)entry + mmap_tag->entry_size)) {

        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            usable_regions[idx].start = entry->addr;
            usable_regions[idx].end   = entry->addr + entry->len;
            idx++;
        }
    }
}

// Remove regions overlapping kernel, early allocator, and multiboot info
static void remove_reserved_regions(void *mb_info) {
    uint64_t kernel_end = (uint64_t)_kernel_end - (uint64_t)kernel_vma();
    uint64_t early_end  = kernel_end + EARLY_ALLOC_SIZE;

    uint64_t mbi_start = (uint64_t)mb_info;
    uint32_t total_size = *(uint32_t *)mb_info;
    uint64_t mbi_end   = mbi_start + total_size;

    uint32_t new_count = 0;

    for (uint32_t i = 0; i < usable_region_count; i++) {
        uint64_t start = usable_regions[i].start;
        uint64_t end   = usable_regions[i].end;

        // Fully covered by kernel/early alloc → discard
        if (end <= early_end)
            continue;

        // Trim overlap with kernel/early alloc
        if (start < early_end && end > early_end)
            start = early_end;

        // Fully inside multiboot info → discard
        if (start >= mbi_start && end <= mbi_end)
            continue;

        // Trim overlap with multiboot info
        if (start < mbi_end && end > mbi_end)
            start = mbi_end;

        if (start < mbi_start && end > mbi_start)
            end = mbi_start;

        if (start >= end)
            continue;

        usable_regions[new_count].start = start;
        usable_regions[new_count].end   = end;

        new_count++;
    }

    usable_region_count = new_count;
}

// Sort regions by start address (simple O(n²))
static void sort_regions(void) {
    for (uint32_t i = 0; i < usable_region_count; i++) {
        for (uint32_t j = i + 1; j < usable_region_count; j++) {
            if (usable_regions[j].start < usable_regions[i].start) {
                memory_region_t tmp = usable_regions[i];
                usable_regions[i] = usable_regions[j];
                usable_regions[j] = tmp;
            }
        }
    }
}

// Merge overlapping or adjacent regions (assumes sorted)
static void merge_regions(void) {
    if (usable_region_count == 0)
        return;

    uint32_t new_count = 0;

    for (uint32_t i = 0; i < usable_region_count; i++) {
        if (new_count == 0) {
            usable_regions[new_count++] = usable_regions[i];
            continue;
        }

        memory_region_t *prev = &usable_regions[new_count - 1];
        memory_region_t *curr = &usable_regions[i];

        // Overlapping or contiguous → merge
        if (prev->end >= curr->start) {
            if (curr->end > prev->end)
                prev->end = curr->end;
        }
        else {
            usable_regions[new_count++] = *curr;
        }
    }

    usable_region_count = new_count;
}

// Normalize = sort + merge
static void normalize_regions(void) {
    sort_regions();
    merge_regions();
}

// Align all regions to page boundaries and drop invalid ones
static void align_regions(void) {
    uint32_t new_count = 0;

    for (uint32_t i = 0; i < usable_region_count; i++) {
        uint64_t start = usable_regions[i].start;
        uint64_t end   = usable_regions[i].end;

        start = align_up(start);   // (addr + 0xFFF) & ~0xFFF
        end   = align_down(end);   // addr & ~0xFFF

        if (start >= end)
            continue;

        usable_regions[new_count].start = start;
        usable_regions[new_count].end   = end;

        new_count++;
    }

    usable_region_count = new_count;
}

// ===== AI-GENERATED (Claude) BEGIN =====
// Carve boot-module ranges out of usable_regions so the page array and PMM
// never touch them (PMM writes a block_t header at each freed page → would
// corrupt the WAD). 모듈 데이터 접근은 vmm_init이 따로 direct-map을 깔아준다.
// 한 region 안에 모듈이 끼어 있으면 두 조각으로 분할한다.
static void reserve_modules(void) {
    if (boot_module_count == 0)
        return;

    // 페이지 정렬된 hole들을 시작주소 오름차순으로 정렬 (단순 삽입정렬)
    uint64_t hs[MAX_BOOT_MODULES];
    uint64_t he[MAX_BOOT_MODULES];
    for (uint32_t i = 0; i < boot_module_count; i++) {
        hs[i] = align_down(boot_modules[i].start);
        he[i] = align_up(boot_modules[i].end);
    }
    for (uint32_t i = 1; i < boot_module_count; i++) {
        uint64_t a = hs[i], b = he[i];
        int32_t j = (int32_t)i - 1;
        while (j >= 0 && hs[j] > a) {
            hs[j + 1] = hs[j];
            he[j + 1] = he[j];
            j--;
        }
        hs[j + 1] = a;
        he[j + 1] = b;
    }

    // 분할로 region이 늘 수 있으므로 새 배열 확보 (모듈당 최대 +1 region)
    uint32_t cap = usable_region_count + boot_module_count;
    memory_region_t *out = early_alloc(cap * sizeof(memory_region_t), 8);
    uint32_t out_count = 0;

    for (uint32_t r = 0; r < usable_region_count; r++) {
        uint64_t cur = usable_regions[r].start;
        uint64_t end = usable_regions[r].end;

        for (uint32_t m = 0; m < boot_module_count && cur < end; m++) {
            if (he[m] <= cur || hs[m] >= end)
                continue;                       // 이 region과 안 겹침
            if (hs[m] > cur && out_count < cap) {
                out[out_count].start = cur;     // hole 앞 조각
                out[out_count].end   = hs[m];
                out_count++;
            }
            if (he[m] > cur)
                cur = he[m];                    // hole 뒤로 진행
        }

        if (cur < end && out_count < cap) {
            out[out_count].start = cur;         // 남은 꼬리 조각
            out[out_count].end   = end;
            out_count++;
        }
    }

    usable_regions      = out;
    usable_region_count = out_count;

    dump_usable_regions("after module reservation");
}
// ===== AI-GENERATED (Claude) END ======

// Entry point: parse multiboot memory map and produce clean usable regions
void multiboot_parse(void *mb_info) {
    struct multiboot_tag *tag;

    tag = (struct multiboot_tag *)((uint8_t *)mb_info + 8);
    // mb_info에는 GRUB이 메모리에 써놓은 구조체의 시작 주소가 들어있음
    // 앞의 8B는 [총 크기 4B][예약 4B]가 들어있음 -> 그래서 mb_info + 8
    // 그 이후에는 태그가 들어있음 (여기서 말하는 태그는 boot.s의 요청 태그와는 다른 것

    // Iterate all multiboot tags
    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
            parse_mmap((struct multiboot_tag_mmap *)tag);
        } else if (tag->type == MULTIBOOT_TAG_TYPE_MODULE) {
            parse_module((struct multiboot_tag_module *)tag);
        } else if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
	    struct multiboot_tag_framebuffer* fb_tag = (struct multiboot_tag_framebuffer*)tag;
	    fb_info.framebuffer_addr	= fb_tag->common.framebuffer_addr;
	    fb_info.framebuffer_pitch	= fb_tag->common.framebuffer_pitch;
	    fb_info.framebuffer_width	= fb_tag->common.framebuffer_width;
	    fb_info.framebuffer_height	= fb_tag->common.framebuffer_height;
	    fb_info.framebuffer_bpp	= fb_tag->common.framebuffer_bpp;
	    fb_info.framebuffer_type	= fb_tag->common.framebuffer_type;
	}

        // Move to next aligned tag (7을 더하고 8바이트 단위로 자름으로써 8바이트 단위 align)
        tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7));
    }

    // Remove reserved areas and normalize
    remove_reserved_regions(mb_info);
    align_regions();
    normalize_regions();

    // Debug output
    dump_usable_regions("normalized usable regions");

    // 부트 모듈(WAD 등) 영역을 usable_regions에서 빼낸다 (AI-GENERATED 호출)
    reserve_modules();
}

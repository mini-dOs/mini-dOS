#include <early_alloc.h>
#include <multiboot2.h>
#include <serial.h>
#include <multiboot.h>

extern uint8_t _end;

memory_region_t *usable_regions;
uint32_t usable_region_count;

// ###################################################################################################
// Debug output
static void serial_write_u32(uint32_t value) {
    char buf[11];
    uint32_t i = 0;

    if (value == 0) {
        serial_write_char('0');
        return;
    }

    while (value > 0) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (i > 0) {
        serial_write_char(buf[--i]);
    }
}

static void dump_usable_regions(const char *label) {
    serial_write("[MMAP] ");
    serial_write(label);
    serial_write(", count=");
    serial_write_u32(usable_region_count);
    serial_write("\n");

    for (uint32_t i = 0; i < usable_region_count; i++) {
        serial_write("[MMAP] region[");
        serial_write_u32(i);
        serial_write("] start=");
        serial_write_hex64(usable_regions[i].start);
        serial_write(" end=");
        serial_write_hex64(usable_regions[i].end);
        serial_write("\n");
    }
}
// ###################################################################################################

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

static uint64_t align_up(uint64_t addr) {
    return (addr + 0xFFF) & ~0xFFF;
}

static uint64_t align_down(uint64_t addr) {
    return addr & ~0xFFF;
}

static void parse_mmap(struct multiboot_tag_mmap *mmap_tag) {
    struct multiboot_mmap_entry *entry;

    usable_region_count = count_usable_regions(mmap_tag);

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

static void remove_reserved_regions(void *mb_info) {
    uint64_t kernel_end = (uint64_t)&_end;
    uint64_t early_end  = kernel_end + EARLY_ALLOC_SIZE;

    uint64_t mbi_start = (uint64_t)mb_info;
    uint32_t total_size = *(uint32_t *)mb_info;
    uint64_t mbi_end   = mbi_start + total_size;

    uint32_t new_count = 0;

    for (uint32_t i = 0; i < usable_region_count; i++) {
        uint64_t start = usable_regions[i].start;
        uint64_t end   = usable_regions[i].end;

        if (end <= early_end)
            continue;

        if (start < early_end && end > early_end)
            start = early_end;

        if (start >= mbi_start && end <= mbi_end)
            continue;

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

static void align_regions(void) {
    uint32_t new_count = 0;

    for (uint32_t i = 0; i < usable_region_count; i++) {
        uint64_t start = usable_regions[i].start;
        uint64_t end   = usable_regions[i].end;

        start = align_up(start);
        end   = align_down(end);

        if (start >= end)
            continue;

        usable_regions[new_count].start = start;
        usable_regions[new_count].end   = end;

        new_count++;
    }

    usable_region_count = new_count;
}

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

static void normalize_regions(void) {
    sort_regions();
    merge_regions();
}

void multiboot_parse(void *mb_info) {
    struct multiboot_tag *tag;

    tag = (struct multiboot_tag *)((uint8_t *)mb_info + 8);

    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
            parse_mmap((struct multiboot_tag_mmap *)tag);
        }

        tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7));
    }

    remove_reserved_regions(mb_info);
    align_regions();
    normalize_regions();

    // Debug output ########################################################
    dump_usable_regions("normalized usable regions");
}
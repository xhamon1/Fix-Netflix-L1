// Spoofs Widevine systemId 30770 -> 30771 (Netflix rejects 30770 for L1).
// Hooks the main executable's AParcel_writeString GOT slot, as in
// Saikrishna1504/device_xiaomi_duchamp@094a6a3, but finds the slot by symbol
// name; falls back to that commit's hardcoded offset on the exact binary it targets.
// No system headers: this builds without an NDK (symbols resolve from bionic's libc at load).
typedef __UINT64_TYPE__ u64; typedef __UINT32_TYPE__ uint32_t; typedef __INT32_TYPE__ int32_t;
typedef __UINTPTR_TYPE__ uintptr_t; typedef __SIZE_TYPE__ size_t;
typedef struct { uint32_t p_type, p_flags; u64 p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_align; } Elf64_Phdr;
typedef struct { long d_tag; union { u64 d_val, d_ptr; } d_un; } Elf64_Dyn;
typedef struct { uint32_t st_name; unsigned char st_info, st_other; unsigned short st_shndx; u64 st_value, st_size; } Elf64_Sym;
typedef struct { u64 r_offset, r_info; long r_addend; } Elf64_Rela;
#define ELF64_R_SYM(i) ((uint32_t)((i) >> 32))
enum { PT_DYNAMIC = 2, PT_PHDR = 6, DT_NULL = 0, DT_PLTRELSZ = 2, DT_STRTAB = 5, DT_SYMTAB = 6, DT_RELA = 7, DT_RELASZ = 8, DT_JMPREL = 23 };
enum { AT_PHDR = 3, AT_PHNUM = 5, PROT_READ = 1, PROT_WRITE = 2 };
unsigned long getauxval(unsigned long);
int mprotect(void*, size_t, int);
int memcmp(const void*, const void*, size_t);
int strcmp(const char*, const char*);

typedef int32_t (*write_fn)(void*, const char*, int32_t);
static write_fn gOrig;

static int32_t Hooked(void* p, const char* s, int32_t n) {
    return gOrig(p, (s && n == 5 && !memcmp(s, "30770", 5)) ? "30771" : s, n);
}

static void install(write_fn* slot) {
    uintptr_t page = (uintptr_t)slot & ~0xfffUL;
    if (mprotect((void*)page, 4096, PROT_READ | PROT_WRITE)) return;
    gOrig = *slot;
    *slot = Hooked;
    mprotect((void*)page, 4096, PROT_READ);
}

// loaders either keep link-time d_ptr values or rewrite them to absolute addresses
#define ADDR(v) ((uintptr_t)(v) >= bias && bias ? (uintptr_t)(v) : bias + (uintptr_t)(v))

static write_fn* find_slot(uintptr_t bias, const Elf64_Dyn* dyn) {
    const Elf64_Sym* sym = 0; const char* str = 0;
    const Elf64_Rela* rel[2] = {0, 0}; size_t sz[2] = {0, 0};
    for (; dyn->d_tag != DT_NULL; dyn++) switch (dyn->d_tag) {
        case DT_SYMTAB:   sym = (const Elf64_Sym*)ADDR(dyn->d_un.d_ptr); break;
        case DT_STRTAB:   str = (const char*)ADDR(dyn->d_un.d_ptr); break;
        case DT_JMPREL:   rel[0] = (const Elf64_Rela*)ADDR(dyn->d_un.d_ptr); break;
        case DT_PLTRELSZ: sz[0] = dyn->d_un.d_val; break;
        case DT_RELA:     rel[1] = (const Elf64_Rela*)ADDR(dyn->d_un.d_ptr); break;
        case DT_RELASZ:   sz[1] = dyn->d_un.d_val; break;
    }
    if (!sym || !str) return 0;
    for (int k = 0; k < 2; k++)
        for (size_t i = 0; rel[k] && i < sz[k] / sizeof(Elf64_Rela); i++) {
            uint32_t si = ELF64_R_SYM(rel[k][i].r_info);
            if (si && !strcmp(str + sym[si].st_name, "AParcel_writeString"))
                return (write_fn*)(bias + rel[k][i].r_offset);
        }
    return 0;
}

__attribute__((constructor)) static void Init(void) {
    const Elf64_Phdr* ph = (const Elf64_Phdr*)getauxval(AT_PHDR);
    size_t n = getauxval(AT_PHNUM);
    if (!ph) return;
    uintptr_t bias = 0;
    const Elf64_Dyn* dyn = 0;
    for (size_t i = 0; i < n; i++) if (ph[i].p_type == PT_PHDR) bias = (uintptr_t)ph - ph[i].p_vaddr;
    for (size_t i = 0; i < n; i++) if (ph[i].p_type == PT_DYNAMIC) dyn = (const Elf64_Dyn*)(bias + ph[i].p_vaddr);
    write_fn* slot = dyn ? find_slot(bias, dyn) : 0;
    if (!slot) {  // shortcut: hardcoded offsets only valid for the binary the upstream commit targets
        uintptr_t base = (uintptr_t)ph & ~0xfffUL;
        if (*(const uint32_t*)(base + 0x320) != 0x2686f259U) return;
        slot = (write_fn*)(base + 0x1aac88);
    }
    install(slot);
}

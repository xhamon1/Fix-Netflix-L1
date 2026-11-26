typedef __UINT64_TYPE__ u64;
typedef __UINT32_TYPE__ u32;
typedef __INT32_TYPE__ i32;
typedef __UINTPTR_TYPE__ uptr;
typedef __SIZE_TYPE__ usize;

typedef struct {
    u32 type, flags;
    u64 offset, vaddr, paddr, filesz, memsz, align;
} Phdr;

typedef struct {
    long tag;
    u64 val;
} Dyn;

typedef struct {
    u32 name;
    unsigned char info, other;
    unsigned short shndx;
    u64 value, size;
} Sym;

typedef struct {
    u64 offset, info;
    long addend;
} Rela;

enum {
    PT_DYNAMIC = 2, PT_PHDR = 6,
    DT_NULL = 0, DT_PLTRELSZ = 2, DT_STRTAB = 5, DT_SYMTAB = 6,
    DT_RELA = 7, DT_RELASZ = 8, DT_JMPREL = 23,
    AT_PHDR = 3, AT_PHNUM = 5,
    PROT_READ = 1, PROT_WRITE = 2
};

unsigned long getauxval(unsigned long);
int mprotect(void *, usize, int);
int memcmp(const void *, const void *, usize);
int strcmp(const char *, const char *);

typedef i32 (*WriteFn)(void *, const char *, i32);

static WriteFn real_write;

static i32 fake_write(void *parcel, const char *s, i32 len)
{
    if (s && len == 5 && !memcmp(s, "30770", 5))
        s = "30771";
    return real_write(parcel, s, len);
}

static void hook(WriteFn *slot)
{
    uptr page = (uptr)slot & ~0xfffUL;

    if (mprotect((void *)page, 4096, PROT_READ | PROT_WRITE))
        return;
    real_write = *slot;
    *slot = fake_write;
    mprotect((void *)page, 4096, PROT_READ);
}

static WriteFn *find_slot(uptr bias, const Dyn *dyn)
{
    const Sym *syms = 0;
    const char *strs = 0;
    const Rela *tab[2] = {0, 0};
    usize size[2] = {0, 0};

    for (; dyn->tag != DT_NULL; dyn++) {
        uptr v = (uptr)dyn->val;
        if (bias && v < bias)
            v += bias;
        switch (dyn->tag) {
        case DT_SYMTAB:   syms = (const Sym *)v; break;
        case DT_STRTAB:   strs = (const char *)v; break;
        case DT_JMPREL:   tab[0] = (const Rela *)v; break;
        case DT_PLTRELSZ: size[0] = dyn->val; break;
        case DT_RELA:     tab[1] = (const Rela *)v; break;
        case DT_RELASZ:   size[1] = dyn->val; break;
        }
    }
    if (!syms || !strs)
        return 0;

    for (int k = 0; k < 2; k++) {
        for (usize i = 0; tab[k] && i < size[k] / sizeof(Rela); i++) {
            u32 idx = (u32)(tab[k][i].info >> 32);
            if (idx && !strcmp(strs + syms[idx].name, "AParcel_writeString"))
                return (WriteFn *)(bias + tab[k][i].offset);
        }
    }
    return 0;
}

__attribute__((constructor)) static void init(void)
{
    const Phdr *ph = (const Phdr *)getauxval(AT_PHDR);
    usize n = getauxval(AT_PHNUM);
    uptr bias = 0;
    const Dyn *dyn = 0;
    WriteFn *slot;

    if (!ph)
        return;
    for (usize i = 0; i < n; i++)
        if (ph[i].type == PT_PHDR)
            bias = (uptr)ph - ph[i].vaddr;
    for (usize i = 0; i < n; i++)
        if (ph[i].type == PT_DYNAMIC)
            dyn = (const Dyn *)(bias + ph[i].vaddr);

    slot = dyn ? find_slot(bias, dyn) : 0;
    if (!slot) {
        uptr base = (uptr)ph & ~0xfffUL;
        if (*(const u32 *)(base + 0x320) != 0x2686f259U)
            return;
        slot = (WriteFn *)(base + 0x1aac88);
    }
    hook(slot);
}

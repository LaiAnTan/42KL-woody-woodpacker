#include "types.h"

// ELF header
typedef struct {
	u8     e_ident[16];         /* Magic number and other info */
	u16    e_type;              /* Object file type */
	u16    e_machine;           /* Architecture */
	u32    e_version;           /* Object file version */
	u64    e_entry;             /* Entry point virtual address */
	u64    e_phoff;             /* Program header table file offset */
	u64    e_shoff;             /* Section header table file offset */
	u32    e_flags;             /* Processor-specific flags */
	u16    e_ehsize;            /* ELF header size in bytes */
	u16    e_phentsize;         /* Program header table entry size */
	u16    e_phnum;             /* Program header table entry count */
	u16    e_shentsize;         /* Section header table entry size */
	u16    e_shnum;             /* Section header table entry count */
	u16    e_shstrndx;          /* Section header string table index */
} t_elf64_hdr;

// program headers
typedef struct {
	u32	p_type;
	u32	p_flags;
	u64	p_offset;
	u64	p_vaddr;
	u64	p_paddr;
	u64	p_filesz;
	u64	p_memsz;
	u64	p_align;
} t_elf64_phdr;

// dynamic entries
typedef struct {
    u64 d_tag;
    union {
        u64 d_val;
        u64 d_ptr;
    } d_un;
} t_elf64_dynhdr;

// relocation table entry
typedef struct {
	u64	r_offset;
	u32	r_type;
	u32 r_sym;
	u64	r_addend;
} t_elf64_rela;

typedef struct
{
	// file structure and metadata
	t_elf64_hdr header;
	t_elf64_phdr *pheaders;
	t_elf64_dynhdr *dynheaders;

	// derived metadata
	char *exec_section;
	int entry_offset;

} t_elf64_info;


int parse_info_elf64(char *buffer, long buffer_size, t_elf64_info* elf_info);
int parse_header_elf64(char *buffer, long buffer_size, t_elf64_hdr *header);
void *map_loadable_program_header_64(t_elf64_hdr elf_hdr, t_elf64_phdr* p_hdrs, char *bin_contents);
void relocate_header_64(t_elf64_info info, char *bin_contents);

// 32 bit land
typedef struct {
	u8     e_ident[16];         /* Magic number and other info */
	u16    e_type;              /* Object file type */
	u16    e_machine;           /* Architecture */
	u32    e_version;           /* Object file version */
	u32    e_entry;             /* Entry point virtual address */
	u32    e_phoff;             /* Program header table file offset */
	u32    e_shoff;             /* Section header table file offset */
	u32    e_flags;             /* Processor-specific flags */
	u16    e_ehsize;            /* ELF header size in bytes */
	u16    e_phentsize;         /* Program header table entry size */
	u16    e_phnum;             /* Program header table entry count */
	u16    e_shentsize;         /* Section header table entry size */
	u16    e_shnum;             /* Section header table entry count */
	u16    e_shstrndx;          /* Section header string table index */
} t_elf32_hdr;

typedef struct {
	u32	p_type;
	u32	p_offset;
	u32	p_vaddr;
	u32	p_paddr;
	u32	p_filesz;
	u32	p_memsz;
	u32	p_flags;
	u32	p_align;
} t_elf32_phdr;

typedef struct
{
	// file structure and metadata
	t_elf32_hdr header;
	t_elf32_phdr pheader;


	// derived metadata
	char *exec_section;
	int entry_offset;
} t_elf32_info;

int parse_header_elf32(char *buffer, long buffer_size, t_elf32_hdr *header);

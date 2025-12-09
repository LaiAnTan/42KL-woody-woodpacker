#include "types.h"

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

t_elf64_hdr *parse_header_elf64(char *buffer, long buffer_size);
t_elf32_hdr *parse_header_elf32(char *buffer, long buffer_size);
#include <string.h>
#include <stdlib.h>

#include "elf.h"
#include "logging.h"

int parse_dynamic_headers_elf64(char *buffer, long buffer_size, t_elf64_dynhdr *dynhdr, int n_headers)
{
	memcpy(dynhdr, buffer, sizeof(t_elf64_dynhdr) * n_headers); 
	return 0;
}


int parse_program_headers_elf64(char *buffer, long buffer_size, t_elf64_phdr *phdr, int n_headers)
{
	// check for size requirement (64 + 56 bytes long / elf hdr length + p hdr length)
	if (buffer_size < (64 + 56) * n_headers)
	{
		error("parsing.parse_program_header_elf64: invalid buffer length");
		return 1;
	}

	buffer = buffer + 64;
	memcpy(phdr, buffer, sizeof(t_elf64_phdr) * n_headers); 
	
	return 0;
}

int parse_info_elf64(char *buffer, long buffer_size, t_elf64_info* elf_info)
{
	t_elf64_hdr header;
	t_elf64_phdr *pheaders;
	t_elf64_dynhdr *dynheaders;


	if (parse_header_elf64(buffer, buffer_size, &header) != 0) {
		return 1;
	}

	// parse program headers
	pheaders = (t_elf64_phdr *)malloc(header.e_phnum * header.e_phentsize);
	if (parse_program_headers_elf64(buffer, buffer_size, pheaders, header.e_phnum) != 0) {
		return 1;
	}

	for (size_t i = 0; i < header.e_phnum; i++)
	{
		debug("parsed Program Headers %zu:", i);
		debug("  p_type:   0x%08x", pheaders[i].p_type);
		debug("  p_flags:  0x%08x", pheaders[i].p_flags);
		debug("  p_offset: 0x%016lx", (unsigned long)pheaders[i].p_offset);
		debug("  p_vaddr:  0x%016lx", (unsigned long)pheaders[i].p_vaddr);
		debug("  p_paddr:  0x%016lx", (unsigned long)pheaders[i].p_paddr);
		debug("  p_filesz: 0x%016lx", (unsigned long)pheaders[i].p_filesz);
		debug("  p_memsz:  0x%016lx", (unsigned long)pheaders[i].p_memsz);
		debug("  p_align:  0x%016lx", (unsigned long)pheaders[i].p_align);
		debug("\n");
	}
	
	// parse dynamic headers
	size_t dyn_offs = 0;
	size_t dyn_offs_sz = 0;
	char *dynamic_section;
	int n_dyn_entries = 0;

	for (size_t i = 0; i < header.e_phnum; i++)
	{
		if (pheaders[i].p_type != 2)
			continue;
		dyn_offs = pheaders[i].p_offset;
		dyn_offs_sz = pheaders[i].p_filesz;
		break; // should only have 1 dyn section in program header, points to only 1 dynamic table offset
	}

	dynheaders = 0;
	if (dyn_offs_sz)
	{
		dynamic_section = buffer + dyn_offs;

		// validate offset size
		if (dyn_offs_sz % sizeof(t_elf64_dynhdr))
		{
			error("relocate_header_64: incorrect dynamic offset size");
			return -1;
		}

		n_dyn_entries = dyn_offs_sz / sizeof(t_elf64_dynhdr);
		dynheaders = (t_elf64_dynhdr *)malloc(dyn_offs_sz);
		if (parse_dynamic_headers_elf64(dynamic_section, 0, dynheaders, n_dyn_entries))
			return 1;

		for (size_t i = 0; i < n_dyn_entries; i++)
		{
			char *start_dyn_entry = dynamic_section + (i * sizeof(t_elf64_dynhdr));
			t_elf64_dynhdr dyn_hdr = dynheaders[i];
			
			debug("parsed dynamic entries %zu:", i);
			debug("  d_tag:   0x%08x", dyn_hdr.d_tag);
			debug("  d_val:  0x%08x", dyn_hdr.d_un.d_val);
			debug("  d_ptr:  0x%08x", dyn_hdr.d_un.d_ptr);
			debug("\n");
		}

	}



	// search for entry offset address, assume its start of a section
	// not doing range search
	int entry_offset = -1;
	for (size_t i = 0; i < header.e_phnum; i++)
	{
		if (pheaders[i].p_type != 1)
			continue;
		size_t start = pheaders[i].p_vaddr;
		size_t end = start + pheaders[i].p_memsz;
		if (header.e_entry < end && header.e_entry >= start)
		{
			size_t remainder = header.e_entry % pheaders[i].p_offset;
			entry_offset = pheaders[i].p_offset + remainder;
		}
	}


	if (entry_offset < 0)
	{
		error("parse_info_elf64: entry offset not found in program header for %x\n", header.e_entry);
		return -1;
	}

	char *exec_section = (char*)((size_t)(buffer + entry_offset));
	
	elf_info->header = header;
	elf_info->pheaders = pheaders;
	elf_info->dynheaders = dynheaders;
	elf_info->exec_section = exec_section;
	elf_info->entry_offset = entry_offset;
	return 0;
}


// NOTE: raw file is stored in big endian for cross compat but 
// implicitly loaded as little endian since my CPU is little endian
// NOTE: assume no extra padding
int parse_header_elf64(char *buffer, long buffer_size, t_elf64_hdr *header)
{
	// check for size requirement (64 bytes long)
	if (buffer_size < 64)
	{
		error("parsing.parse_header_elf64: invalid buffer length");
		return 1;
	}

	memcpy(header, buffer, sizeof(t_elf64_hdr)); 
	
	return 0;
}




// ELF 32 land


// NOTE: assume no extra padding and memory layout is little endian
int parse_header_elf32(char *buffer, long buffer_size, t_elf32_hdr *header)
{
	// check for size requirement (58 bytes long)
	if (buffer_size < 58)
	{
		error("parsing.parse_header_elf32: invalid buffer length");
		return 1;
	}

	memcpy(header, buffer, sizeof(t_elf32_hdr)); 
	
	return 0;
}
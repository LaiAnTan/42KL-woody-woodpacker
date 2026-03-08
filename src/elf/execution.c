#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>

#include "elf.h"
#include "logging.h"

// for every loadable program header, malloc and copy as void ptr
void *map_loadable_program_header_64(t_elf64_hdr elf_hdr, t_elf64_phdr* p_hdrs, char *bin_contents)
{
	int sizes = 0;	
	void **res = malloc(sizeof(void*) * elf_hdr.e_phnum);

	for (size_t i = 0; i < elf_hdr.e_phnum; i++)
	{
		// if we have non loadable program header, skip mapping as well
		// or skip empty range mappings
		if (p_hdrs[i].p_type != 1 || p_hdrs[i].p_memsz == 0)
		{
			info("pheader %i not mapped due to non load", i);
			res[i] = 0;
			continue;
		}


		// TODO: make this safer
		void *mapped_addr = mmap((void *) p_hdrs[i].p_vaddr, p_hdrs[i].p_memsz, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
		// info("woah 0x%08x, 0x%08x, %x", mapped_addr,  (void *)(bin_contents + p_hdrs[i].p_offset),  p_hdrs[i].p_filesz);
		memcpy(mapped_addr,  (void *)(bin_contents + p_hdrs[i].p_offset),  p_hdrs[i].p_filesz);
		mprotect(mapped_addr,  p_hdrs[i].p_filesz, p_hdrs[i].p_flags);
		info("mapped and copied addr 0x%016x - 0x%016x to 0x%016x - 0x%016x %s", p_hdrs[i].p_vaddr, p_hdrs[i].p_vaddr + p_hdrs[i].p_memsz, mapped_addr, mapped_addr + p_hdrs[i].p_memsz, strerror(errno));
		res[i] = mapped_addr;
	}
	return res;
}

// do dynamic relocations
void relocate_header_64(t_elf64_info elf_info, char *bin_contents)
{
	size_t dyn_offs_sz = 0;
	int n_dyn_entries = 0;
	t_elf64_phdr *pheaders = elf_info.pheaders;
	t_elf64_dynhdr* dynheaders = elf_info.dynheaders;
	t_elf64_hdr elf_header = elf_info.header;

	if (!dynheaders)
	{
		info("relocate_header_64: no headers, skipping relocation");
		return;
	}
	
	for (size_t i = 0; i < elf_header.e_phnum; i++)
	{
		if (pheaders[i].p_type != 2)
			continue;
		dyn_offs_sz = pheaders[i].p_filesz;
		break; // should only have 1 dyn section in program header, points to only 1 dynamic table offset
	}
	n_dyn_entries = dyn_offs_sz / sizeof(t_elf64_dynhdr);

	// find relavent information
	// dyn type Rela, RelaSz, RelaEnt, RelACount
	int rela_count = 0;
	int rela_size_unit = 0;
	int rela_size_total = 0;
	int rela_addr = 0;
	for (size_t i = 0; i < n_dyn_entries; i++)
	{
		t_elf64_dynhdr curr_dyn_hdr = dynheaders[i];
		if (curr_dyn_hdr.d_tag == 0x8)
		{
			// info("RELASZ found, val %x", curr_dyn_hdr.d_un.d_val);
			rela_size_total = curr_dyn_hdr.d_un.d_val;
		}
		if (curr_dyn_hdr.d_tag == 0x9)
		{
			// info("RELASZENT found, val %x", curr_dyn_hdr.d_un.d_val);
			rela_size_unit = curr_dyn_hdr.d_un.d_val;
		}
		if (curr_dyn_hdr.d_tag == 0x7)
		{
			// info("RELA found, val %x", curr_dyn_hdr.d_un.d_ptr);
			rela_addr = curr_dyn_hdr.d_un.d_ptr;
		}
	}

	if (rela_addr == 0)
		return;
	
	rela_count = rela_size_total / rela_size_unit;

	// locate the offset where the rela_addr lives via loadable program header
	u64 rela_file_offset = 0;
	for (size_t i = 0; i < elf_header.e_phnum; i++)
	{
		if (pheaders[i].p_type != 0x1)
			continue;
		u64 start_addr = pheaders[i].p_vaddr;
		u64 end_addr = pheaders[i].p_vaddr + pheaders[i].p_memsz;
		if (rela_addr < end_addr && rela_addr > start_addr)
		{
			rela_file_offset = pheaders[i].p_offset + rela_addr;
			break;
		}
	}

	t_elf64_rela *relo_table = malloc(sizeof(t_elf64_rela) * rela_count);
	memcpy(relo_table, bin_contents + rela_file_offset, sizeof(t_elf64_rela) * rela_count); 

	// for each relocation table entry, read
	for (size_t i = 0; i < rela_count; i++)
	{
		t_elf64_rela relo_entry = relo_table[i];

		// locate the offset where the virtual address stated in relocation entry lives via loadable program header
		u64 relocatable_addr = 0;
		for (size_t j = 0; j < elf_header.e_phnum; j++)
		{
			if (pheaders[j].p_type != 0x1)
				continue;
			u64 start_addr = pheaders[j].p_vaddr;
			u64 end_addr = pheaders[j].p_vaddr + pheaders[j].p_memsz;
			if (relo_entry.r_offset < end_addr && relo_entry.r_offset > start_addr)
			{
				relocatable_addr = pheaders[i].p_offset + relo_entry.r_offset;
				break;
			}
		}

		// handle type RELATIVE relocations here
		info("relo entry %d, offset %lx, type %u, sym %u, addend %lx", 
			i, relo_entry.r_offset, relo_entry.r_type, relo_entry.r_sym, relo_entry.r_addend);
		if (relo_entry.r_type == 0x8){
			// NOTE: assume we are working with relocatable fiels but not exec here...
			// char *to_replace = bin_contents + relocatable_addr;
			char *to_replace = bin_contents + relo_entry.r_offset;
			// u64 *to_replace_int = malloc(sizeof(u64));
			// memcpy(to_replace_int, to_replace, sizeof(u64)); 
			// info("i will now change %x at %x with %x", *to_replace_int, relocatable_addr, bin_contents + relo_entry.r_addend);
			// free(to_replace_int);
			
			// char *replace_with = bin_contents + relo_entry.r_addend;
			u64 replace_with = (u64) bin_contents + relo_entry.r_addend;
			memcpy(to_replace, &replace_with, sizeof(u64));
			
	}
	
	// memory offset and reloacation type
	// based on relocation type, change contents in bin_contents
	// to apply relocation


	free(relo_table);
	return;
}
#include <string.h>
#include <stdlib.h>

#include "ft_elf.h"
#include "logging.h"

int get_text_section_header(Elf64_Shdr **text_section_ptr, t_elf_info elf_info)
{
	// find .shstrtab SHT_STRTAB section to locate the string table
	Elf64_Shdr *sections = elf_info.sections;
	int num_sections = elf_info.elf_header->e_shnum;
	
	for (size_t i = 0; i < num_sections; i++)
	{
		Elf64_Shdr curr_section = sections[i];
		info("mid %p %p %p", &curr_section, curr_section, sections);
		if (curr_section.sh_type == SHT_STRTAB)
		{
			Elf32_Word sh_idx = curr_section.sh_name;
			Elf32_Off sh_offset = curr_section.sh_offset;
			Elf32_Off sh_size = curr_section.sh_size;
			
			unsigned char *sect_name = elf_info.guest_file.contents + sh_offset + sh_idx;
			// once section name string table is located, find text section
			if (!strcmp((const char *) sect_name, ".shstrtab"))
			{
				for (size_t j = 0; j < num_sections; j++)	
				{
					Elf64_Shdr query_section = sections[j];
					Elf32_Word query_sh_idx = query_section.sh_name;
					unsigned char *query_sect_name = elf_info.guest_file.contents + sh_offset + query_sh_idx;
					
					if (!strcmp((const char *) query_sect_name, ".text"))
					{
						*text_section_ptr = &query_section;
						return 0;
					}
				}
				
				error("get_text_section_header: .text section not found");
				return 1;
			}
		}
	}

	error("get_text_section_header: shstrtab section not found");
	return 1;
}

int init_elf_info(t_elf_info *elf_info, t_file_info guest_file)
{
	elf_info->guest_file = guest_file;
	elf_info->elf_header = guest_file.contents;

	// check magic
	if (elf_info->elf_header->e_ident[0] != 0x7f ||
		elf_info->elf_header->e_ident[1] != 'E' ||	
		elf_info->elf_header->e_ident[2] != 'L' ||
		elf_info->elf_header->e_ident[3] != 'F' 
	)
	{
		error("init_elf_info: invalid magic");
		return 1;
	}

	// check 64 bit
	if (elf_info->elf_header->e_ident[4] != 0x2)
	{
		error("init_elf_info: invalid file class");
		return 1;
	}

	// check valid header offsets
	if ((long)elf_info->elf_header->e_phoff > guest_file.size || (long)elf_info->elf_header->e_shoff > guest_file.size)
	{
		error("init_elf_info: invalid header offsets");
		return 1;		
	}

	if ((long)elf_info->elf_header->e_phoff == 0 || (long)elf_info->elf_header->e_shoff == 0)
	{
		error("init_elf_info: no program header or no section header");
		return 1;
	}

	// fill up the rest of elf_info
	elf_info->segments = guest_file.contents + elf_info->elf_header->e_phoff;
	elf_info->sections = guest_file.contents + elf_info->elf_header->e_shoff;

	info("before %p", elf_info->text_section);
	get_text_section_header(&elf_info->text_section, *elf_info);
	info("after %x %x %x", elf_info->text_section, elf_info->guest_file.contents, elf_info->guest_file.contents + elf_info->guest_file.size);




	return 0;
}
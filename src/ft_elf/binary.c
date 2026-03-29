#include <string.h>
#include "ft_elf.h"
#include "enc.h"
#include "logging.h"
#include "types.h"
#include "shellcode.h"

void *write_until_entry(t_elf_info *elf_info, void *start, unsigned char **stub_buffer, Elf64_Addr new_entry)
{
	// fill up everything in stub buffer from the old elf with a size of n bytes UNTIL the new entry address should be
	unsigned long start_to_entry_sz = (unsigned long) &(elf_info->elf_header->e_entry) - (unsigned long) start;
	memcpy(*stub_buffer, start,  start_to_entry_sz);
	*stub_buffer += start_to_entry_sz;

	// replace new_entry with the new entry point we just malloced
	memcpy(*stub_buffer, &new_entry, sizeof(new_entry));
	*stub_buffer += sizeof(new_entry);

	// move start pointer to point to space after new_entry is defined
	start = (void *)&(elf_info->elf_header->e_entry) + sizeof(new_entry);
	
	return start;
}

void *include_shellcode_sz_in_segment(void *start, unsigned char **stub_buffer, Elf64_Phdr *segment, t_key *key)
{
	// copy elements until the current segments file_sz
	memcpy(*stub_buffer, start, (unsigned long)&segment->p_filesz - (unsigned long)start);
	*stub_buffer += (unsigned long)&segment->p_filesz - (unsigned long)start;
	start = &segment->p_filesz;

	// generate new p_filesze which will include SHELLCODE_SIZE and keysize
	uint64_t new_p_filesz = segment->p_filesz + SHELLCODE_SIZE + key->size;
	uint64_t new_p_memsz = segment->p_memsz + SHELLCODE_SIZE + key->size;

	// copy filesz and mem pz to stub
	// NOTE: does not override guest filesz and memsz here... we need them
	// for shellcode injection later
	memcpy(*stub_buffer, &new_p_filesz, sizeof(segment->p_filesz));
	*stub_buffer += sizeof(segment->p_filesz);
	start += sizeof(segment->p_filesz);

	memcpy(*stub_buffer, &new_p_memsz, sizeof(segment->p_memsz));
	*stub_buffer += sizeof(segment->p_memsz);
	start += sizeof(segment->p_memsz);

	return start;
}

void *update_segment_offsets(t_elf_info *elf_info, void *start, unsigned char **stub_buffer, t_key *key)
{
	Elf64_Off		new_shoff;

	// create new section header offset, which is the original offset + PAGE_SIZE (padding)
	// and copy until shoff position and write new shoff
	new_shoff = elf_info->elf_header->e_shoff + PAGE_SIZE;
	unsigned long start_to_eshoff_sz = (unsigned long) &(elf_info->elf_header->e_shoff) - (unsigned long) start;
	memcpy(*stub_buffer, start,  start_to_eshoff_sz);
	*stub_buffer += start_to_eshoff_sz;
	memcpy(*stub_buffer, &new_shoff, sizeof(new_shoff));
	*stub_buffer += sizeof(new_shoff);
	start = (void *)&(elf_info->elf_header->e_shoff) + sizeof(new_shoff);

	// upadate all other segments that comes after the init load segment
	// to increment PAGE_SIZE (padding) in their offset
	for (int i = 0; i < elf_info->elf_header->e_phnum; i++)
	{
		// if current segment is first loadable segment
		if ((unsigned long)&elf_info->segments[i] == (unsigned long)elf_info->pt_load)
			start = include_shellcode_sz_in_segment(start, stub_buffer, elf_info->pt_load, key); // write updated file and mem sizes 
		else if (elf_info->segments[i].p_offset >= (unsigned long)elf_info->pt_load->p_offset + elf_info->pt_load->p_filesz)
		{
			Elf64_Off new_p_off = elf_info->segments[i].p_offset + PAGE_SIZE;
			memcpy(*stub_buffer, start, (unsigned long)&elf_info->segments[i].p_offset - (unsigned long)start);
			*stub_buffer += (unsigned long)&elf_info->segments[i].p_offset - (unsigned long)start;
			memcpy(*stub_buffer, &new_p_off, sizeof(new_p_off));
			*stub_buffer += sizeof(new_p_off);
			start = (void *)&elf_info->segments[i].p_offset + sizeof(elf_info->segments[i].p_offset);
		}
	}

	return start;
}

int write_enc_text_section(t_elf_info *elf_info, void *start, unsigned char **stub_buffer, t_key *key)
{
	uint64_t txt_sect_offs = elf_info->text_section->sh_offset;
	uint64_t txt_sect_size = elf_info->text_section->sh_size; 
	uint64_t start_to_txt_scn_sz = (unsigned long)(elf_info->guest_file.contents + txt_sect_offs) - (unsigned long)start;
	char *encrypted_text = xor_encrypt(elf_info->guest_file.contents + txt_sect_offs, txt_sect_size, key);
	if (!encrypted_text)
		return (1);	
	
	memcpy(*stub_buffer, start, start_to_txt_scn_sz);
	*stub_buffer += start_to_txt_scn_sz;
	
	memcpy(*stub_buffer, encrypted_text, txt_sect_size);	
	*stub_buffer += txt_sect_size;

	return 0;
}

int write_binary(unsigned char **stub_buffer, t_key *key, t_file_info *guest_file, t_elf_info *elf_info)
{
	// new entry point to write in stub
	Elf64_Addr	new_entry = elf_info->pt_load->p_vaddr + elf_info->pt_load->p_memsz;
	void	*start = guest_file->contents;
	void	*end = guest_file->contents + guest_file->size;

	// write the elf header until new entry
	start = write_until_entry(elf_info, start, stub_buffer, new_entry);

	// add the segment padding for shellcode segment
	// and adjust offset for the sections after the padded section
	start = update_segment_offsets(elf_info, start, stub_buffer, key);

	// 'start' should be at the end of the program header now
	// copy until the text section of the program and write the encrypted text section
	int ret = write_enc_text_section(elf_info, start, stub_buffer, key);
	if (ret)
		return ret;

	// update start because we didnt update jn...
	// update start to end of text section
	start += (unsigned long)(elf_info->guest_file.contents + elf_info->text_section->sh_offset) - (unsigned long)start + (unsigned long)elf_info->text_section->sh_size;

	// copy everything else thats left for the first segment
	// which should be the segment which includes the original text section
	uint64_t start_to_end_seg_sz = (unsigned long)(elf_info->guest_file.contents + elf_info->pt_load->p_offset) - (unsigned long)start + (unsigned long)elf_info->pt_load->p_memsz;
	memcpy(*stub_buffer, start, start_to_end_seg_sz);
	start += start_to_end_seg_sz;
	*stub_buffer += start_to_end_seg_sz;	

	


	return 0;
}
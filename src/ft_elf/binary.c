#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ft_elf.h"
#include "enc.h"
#include "logging.h"
#include "types.h"
#include "shellcode.h"

void *write_shellcode_and_padding(t_elf_info *elf_info, void *start, unsigned char **stub_buffer, Elf64_Addr new_entry, t_key *key)
{
	// write shellcode and key
	// for (size_t i = 0; i < SHELLCODE_SIZE; i++)
	// {
	// 	printf("%x, ", SHELLCODE_BYTES[i]);
	// }
	// printf("\n");
	
	memcpy(*stub_buffer, SHELLCODE_BYTES, SHELLCODE_SIZE);
	*stub_buffer += SHELLCODE_SIZE;
	memcpy(*stub_buffer, key->buffer, key->size);
	*stub_buffer += key->size;

	// fill page aligned padding

	// Elf64_Phdr *next_segment = elf_info->pt_load + 1;
	// unsigned int padding_len = (SHELLCODE_SIZE + key->size) - (next_segment->p_offset - (elf_info->pt_load->p_offset + elf_info->pt_load->p_filesz));
	// unsigned int padding_len_page_aligned =	PAGE_SIZE - (padding_len % PAGE_SIZE);
	
	// int diff = (SHELLCODE_SIZE + key->size) - (next_segment->p_offset - (elf_info->pt_load->p_offset + elf_info->pt_load->p_filesz));
	// int diff_2 = PAGE_SIZE - (diff % PAGE_SIZE);
	
	// // memset(*stub_buffer, 0, padding_len_page_aligned);
	// // *stub_buffer += padding_len_page_aligned;
	// memset(*stub_buffer, 0, diff_2);
	// *stub_buffer += diff_2;
	// info("how many EMPTY bytes from first segment offset to next segment offset %d", next_segment->p_offset - (elf_info->pt_load->p_offset + elf_info->pt_load->p_filesz));
	// info("how many EMPTY bytes from first segment offset to next segment offset %d", next_segment->p_offset - (elf_info->pt_load->p_offset + elf_info->pt_load->p_filesz));
	
	// TODO: handle this difference by filling actual correct padding, also, make sure nexy segment has offset value in validation (?)
	// offset difference should be the original difference + PAGE_SIZE
	// in other words, new next segment offset will be old next_segment->p_offset + PAGE_SIZE * already made sure
	// everything after curr_filled to new next segment offset will be filled with 0

	Elf64_Phdr *next_segment = elf_info->pt_load + 1;
	unsigned int next_segment_offset = next_segment->p_offset;
	unsigned int curr_offset = elf_info->pt_load->p_offset + elf_info->pt_load->p_filesz;
	unsigned int curr_filled = curr_offset + SHELLCODE_SIZE + key->size;
	unsigned int new_next_segment_offset = next_segment_offset + PAGE_SIZE;
	unsigned int size_to_next_offset = new_next_segment_offset - curr_filled;
	info("next_segment_offset %d", next_segment_offset);
	info("curr_offset %d", curr_offset);
	info("curr_filled %d", curr_filled);
	info("new_next_segment_offset %d", new_next_segment_offset);
	info("size_to_next_offset %d", size_to_next_offset);
	info("PAGE_SIZE %d", PAGE_SIZE);
	memset(*stub_buffer, 0, size_to_next_offset);	
	*stub_buffer += size_to_next_offset;


	start = elf_info->guest_file.contents + next_segment->p_offset;

	return start;
}

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

void *update_section_offsets(t_elf_info *elf_info, void *start, unsigned char **stub_buffer)
{
	Elf64_Off		new_shoff;

	// copy the rest of the things here


	for (int i = 0; i < elf_info->elf_header->e_shnum; i++)
	{
		if ((unsigned long)elf_info->sections[i].sh_offset > (unsigned long)elf_info->pt_load->p_offset + elf_info->pt_load->p_filesz)
		{
			new_shoff = elf_info->sections[i].sh_offset + PAGE_SIZE;
			// info("stub buffer curr %p", *stub_buffer);
			// info("updating shoff at offset %x", (unsigned long)&elf_info->sections[i].sh_offset - (unsigned long) elf_info->guest_file.contents);
			memcpy(*stub_buffer, start, ((unsigned long)&elf_info->sections[i].sh_offset - (unsigned long)start));
			*stub_buffer += (unsigned long)&elf_info->sections[i].sh_offset - (unsigned long)start;
			memcpy(*stub_buffer, &new_shoff, sizeof(new_shoff));
			*stub_buffer += sizeof(new_shoff);
			start = (void *)&elf_info->sections[i].sh_offset + sizeof(elf_info->sections[i].sh_offset);
		}
	}
	return start;
}

void *update_segment_offsets(t_elf_info *elf_info, void *start, unsigned char **stub_buffer, t_key *key)
{
	Elf64_Off		new_shoff;

	// create new section header offset, which is the original offset + PAGE_SIZE (padding)
	// and copy until shoff position and write new shoff
	info("old shoff %x, total size %x, max off %x", elf_info->elf_header->e_shoff, elf_info->elf_header->e_shentsize * elf_info->elf_header->e_shnum, elf_info->elf_header->e_shoff + elf_info->elf_header->e_shentsize * elf_info->elf_header->e_shnum);
	new_shoff = elf_info->elf_header->e_shoff + PAGE_SIZE;
	info("new shoff %x", new_shoff);
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

	// for (size_t i = 0; i < txt_sect_size; i++)
	// {
	// 	printf("%x, ", encrypted_text[i]);
	// }
	// printf("\n");
	
	memcpy(*stub_buffer, start, start_to_txt_scn_sz);
	*stub_buffer += start_to_txt_scn_sz;
	
	memcpy(*stub_buffer, encrypted_text, txt_sect_size);	
	*stub_buffer += txt_sect_size;

	free(encrypted_text);
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
	info("entry offset %x", start - guest_file->contents);
	info("new entry %x", new_entry);


	// add the segment padding for shellcode segment
	// and adjust offset for the sections after the padded section
	start = update_segment_offsets(elf_info, start, stub_buffer, key);
	info("pheader end offset %x", start - guest_file->contents);

	// 'start' should be at the end of the program header now
	// copy until the text section of the program and write the encrypted text section
	int ret = write_enc_text_section(elf_info, start, stub_buffer, key);
	if (ret)
		return ret;

	// update start because we didnt update jn...
	// update start to end of text section
	start += (unsigned long)(elf_info->guest_file.contents + elf_info->text_section->sh_offset) - (unsigned long)start + (unsigned long)elf_info->text_section->sh_size;
	info("stub buffer curr %p", *stub_buffer);
	info("end enc offset %x", start - guest_file->contents);

	// copy everything else thats left for the first segment
	// which should be the segment which includes the original text section
	uint64_t start_to_end_seg_sz = (unsigned long)(elf_info->guest_file.contents + elf_info->pt_load->p_offset) - (unsigned long)start + (unsigned long)elf_info->pt_load->p_memsz;
	memcpy(*stub_buffer, start, start_to_end_seg_sz);
	start += start_to_end_seg_sz;
	*stub_buffer += start_to_end_seg_sz;
	info("segment remaininig bytes %x", start_to_end_seg_sz);
	info("stub buffer curr %p", *stub_buffer);
	info("end 1st segment offset %x", start - guest_file->contents);


	// write shellcode and key at current position (segment)	
	// and fill padding until end of current segment
	start = write_shellcode_and_padding(elf_info, start, stub_buffer, new_entry, key);
	info("stub buffer curr %p", *stub_buffer);
	info("end shellcode padding offset %x", start - guest_file->contents);

	// adjust the offset for the section headers
	start = update_section_offsets(elf_info, start, stub_buffer);

	// write the rest of the elf
	memcpy(*stub_buffer, start, end - start);

	return 0;
}
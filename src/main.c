#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <string.h>
#include <sys/mman.h>

#include "logging.h"
#include "io.h"
#include "elf.h"
#include "LzmaEnc.h"
#include "LzmaDec.h"
#include "Alloc.h"

#define MAX_FILE_SIZE (size_t) 1024 * 1024 * 1024 * 5

int validate_args(int argc, char const *argv[])
{
	// TODO: add more inputs for parameterized key bonus
	if (argc != 2)
	{
		printf("usage: woody <file_name> <aes-key-or-whatever>\n");
		return 1;
	}
	return 0;
}



int main(int argc, char const *argv[])
{
	if (validate_args(argc, argv))
		return 1;

	char* alloc_file_contents = malloc(MAX_FILE_SIZE); // 5MB allocation
	const char* file_name = argv[1];
	long file_size = read_file(file_name, alloc_file_contents, MAX_FILE_SIZE);
	if ( file_size < 0)
		return -1;

	t_elf64_info elf_info;
	int info_res = parse_info_elf64(alloc_file_contents, file_size, &elf_info);
	if (info_res)
		return -1;
	t_elf64_hdr header = elf_info.header;

	// create mmap
	map_loadable_program_header_64(elf_info.header, elf_info.pheaders, alloc_file_contents);

	// dynamic relocation
	relocate_header_64(elf_info, alloc_file_contents);

	char *exec_section = elf_info.exec_section;
	long exec_section_len = file_size - elf_info.entry_offset;

	info("exec_section is at %p, offset %p", exec_section, elf_info.entry_offset);
	int ret = mprotect((void *)((size_t)exec_section & (~0xFFF)), exec_section_len, PROT_READ | PROT_EXEC | PROT_WRITE);// // memory align for mprotect, addr must end with page size -- 3 0x0s
	info("exec_section is at %p, ret %d, errno %s", exec_section, ret, strerror(errno));

	void (*jump_target)(void) = (void (*)(void))exec_section;
	jump_target();


	// FILE *file = fopen("out.bin", "wb");  // Create/truncate binary file
    // if (!file) return 1;
    // size_t written = fwrite(exec_section, 1, exec_section_len, file);  // size=1 byte, count=4
    // fclose(file);
	// info("written %ld/%ld bytes", written, exec_section_len);


	// CLzmaEncProps props;
	// LzmaEncProps_Init(&props);

	free(alloc_file_contents);
	return 0;
}

#include <stdio.h>
#include <stdlib.h>

#include "logging.h"
#include "io.h"
#include "ft_elf.h"
#include "LzmaEnc.h"
#include "LzmaDec.h"
#include "Alloc.h"
#include "shellcode.h"

#define MAX_FILE_SIZE (size_t) 1024 * 1024 * 1024 * 5

int validate_args(int argc, char const *argv[])
{
	// TODO: add more inputs for parameterized key bonus
	if (argc != 2)
	{
		error("usage: woody <file_name> <aes-key-or-whatever>\n");
		return 1;
	}
	return 0;
}

int main(int argc, char const *argv[])
{
	if (validate_args(argc, argv))
		return 1;

	// for (size_t i = 0; i < SHELLCODE_SIZE; i++)
	// {
	// 	printf("%x ", SHELLCODE_BYTES[i]);
	// }
	// printf("\n");

	// read file contents using mmap
	t_file_info guest_file;
	int ret = read_file(argv[1], &guest_file);
	if (ret)
		return ret;

	// parse elf_info
	t_elf_info elf_info;
	ret = init_elf_info(&elf_info, guest_file);
	if (ret)
		return ret;

	return 0;
}

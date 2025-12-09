#include <stdio.h>
#include <stdlib.h>

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

	t_elf64_hdr *test = parse_header_elf64(alloc_file_contents, file_size);
	info("%x, %x, %x", test->e_ident, test->e_type, test->e_shstrndx);

	// CLzmaEncProps props;
	// LzmaEncProps_Init(&props);

	free(test);
	free(alloc_file_contents);
	return 0;
}

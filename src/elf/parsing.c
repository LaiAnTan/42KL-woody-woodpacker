#include <string.h>
#include <stdlib.h>

#include "elf.h"
#include "logging.h"

// NOTE: assume no extra padding and memory layout is little endian
t_elf64_hdr *parse_header_elf64(char *buffer, long buffer_size)
{
	// check for size requirement (64 bytes long)
	if (buffer_size < 64)
	{
		error("parsing.parse_header_elf64: invalid buffer length");
		return 0;
	}

	t_elf64_hdr *res = malloc(sizeof(t_elf64_hdr));
	memcpy(res, buffer, sizeof(t_elf64_hdr)); 
	
	return res;
}

// NOTE: assume no extra padding and memory layout is little endian
t_elf32_hdr *parse_header_elf32(char *buffer, long buffer_size)
{
	// check for size requirement (58 bytes long)
	if (buffer_size < 58)
	{
		error("parsing.parse_header_elf32: invalid buffer length");
		return 0;
	}

	t_elf32_hdr *res = malloc(sizeof(t_elf32_hdr));
	memcpy(res, buffer, sizeof(t_elf32_hdr)); 
	
	return res;
}
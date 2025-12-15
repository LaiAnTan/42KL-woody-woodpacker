#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "logging.h"
#include "io.h"

long read_file(const char *file_name, char *buffer, long max_size)
{
	int fd = open(file_name, O_RDONLY);
	long read_buf_passed = 0;
	int read_buf_size = 1024;

    if (fd == -1) {
		char *errmsg = strerror(errno);
		error("io.read_file: %s", errmsg);
		return -1;
    }

	while (read_buf_passed < max_size) {
		int read_bytes = read(fd, buffer + read_buf_passed, read_buf_size);
		if (read_bytes == 0 || read_bytes == -1)
			break;
		read_buf_passed += read_bytes;
	}

	if (read_buf_passed >= max_size)
	{
		error("io.read_file: max size reached");
		return -67;
	}

	return read_buf_passed;
}

int determine_exec_file_type(char *buffer, long buffer_size)
{
	// auto return error if buffer is too small for magic number scanning
	if (buffer_size < 10)
	{
		error("io.determine_exec_file_type: file too small");
		return -1;
	}

	// check for ELF magic number
	if (buffer[0] == 0x7F && buffer[1] == 'E' && buffer[2] == 'L' && buffer[3] == 'F')
	{
		// TODO: only handle little endian files
		if (buffer[5] == 0x02)
		{
			error("io.determine_exec_file_type: unsupported endianess");
			return -1;	
		}

		// check for 32 or 64 bit
		if (buffer[4] == 1)
			return FILE_TYPE_ELF_32;
		return FILE_TYPE_ELF_64;
	}

	// check for macho magic number
	if (buffer[0] == 0xcf && buffer[1] == 0xfa && buffer[2] == 0xed && buffer[3] == 0xfe)
		return FILE_TYPE_MACHO;

	error("io.determine_exec_file_type: unsupported file");
	return -1;
}
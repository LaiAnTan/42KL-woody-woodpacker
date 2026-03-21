#ifndef __IO__H__
#define __IO__H__
#define FILE_TYPE_ELF_64 0
#define FILE_TYPE_ELF_32 1
#define FILE_TYPE_MACHO 2

typedef struct file_info
{
	int fd;
	int size;
	void* contents;
} t_file_info;


// long read_file(const char* file_name, char *buffer, long max_size);
int read_file(const char* file_name, t_file_info *file);
int determine_exec_file_type(unsigned char *buffer, long buffer_size);

#endif  //!__IO__H__
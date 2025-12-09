#define FILE_TYPE_ELF_64 0
#define FILE_TYPE_ELF_32 1
#define FILE_TYPE_MACHO 2

long read_file(const char* file_name, char *buffer, long max_size);
int determine_exec_file_type(char *buffer, long buffer_size);
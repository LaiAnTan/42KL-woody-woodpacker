add_library(elf)

target_sources(elf
	PRIVATE
		execution.c
    	parsing.c

	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS
		.
		FILES
		elf.h
)

target_link_libraries(elf
  PRIVATE
    types
	logging
)
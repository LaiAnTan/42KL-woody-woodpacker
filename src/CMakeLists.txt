add_compile_options(-fsanitize=address -g3)
add_link_options(-fsanitize=address -g3)

add_executable(woody)

add_subdirectory(types)
add_subdirectory(io)
add_subdirectory(elf)
add_subdirectory(aes)
add_subdirectory(lzma)
add_subdirectory(logging)

target_sources(woody
	PRIVATE main.c
)

target_link_libraries(woody
  PRIVATE
    logging
	lzma
	io
	types
	elf
)

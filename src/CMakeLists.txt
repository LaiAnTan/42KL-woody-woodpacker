add_compile_options(-fsanitize=address -Wall -Wextra -g3) # TODO: add warning flags here
add_link_options(-fsanitize=address -Wall -Wextra -g3)

add_executable(woody_woodpacker)

add_subdirectory(types)
add_subdirectory(io)
add_subdirectory(ft_elf)
add_subdirectory(enc)
add_subdirectory(lzma)
add_subdirectory(logging)
add_subdirectory(shellcode)

target_sources(woody_woodpacker
	PRIVATE main.c
)

target_link_libraries(woody_woodpacker
  PRIVATE
    logging
	lzma
	io
	types
	ft_elf
	shellcode
	enc
)

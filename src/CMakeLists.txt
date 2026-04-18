add_compile_options(-fsanitize=address -Wall -Wextra -g3)
add_link_options(-fsanitize=address -Wall -Wextra -g3)

add_executable(woody_woodpacker)

add_subdirectory(types)
add_subdirectory(io)
add_subdirectory(ft_elf)
add_subdirectory(enc)
add_subdirectory(logging)
add_subdirectory(shellcode)
add_subdirectory(libft)

target_sources(woody_woodpacker
	PRIVATE main.c
)

target_link_libraries(woody_woodpacker
  PRIVATE
    logging
	io
	types
	ft_elf
	shellcode
	enc
	libft
)

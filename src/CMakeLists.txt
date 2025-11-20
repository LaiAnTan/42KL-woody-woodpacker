add_executable(woody)

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
)

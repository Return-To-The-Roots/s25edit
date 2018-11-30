FIND_PACKAGE(SDL REQUIRED)

file(GLOB SGE_SOURCES *.cpp *.h)
add_library(SGE STATIC ${SGE_SOURCES})
target_link_libraries(SGE PUBLIC ${SDL_LIBRARY})
target_include_directories(SGE SYSTEM PUBLIC ${SDL_INCLUDE_DIR})
if(ClangFormat_FOUND)
    add_clangFormat_files(${SGE_SOURCES})
endif()

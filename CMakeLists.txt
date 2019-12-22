cmake_minimum_required(VERSION 3.8)

project(s25edit)

find_package(Boost 1.64 REQUIRED)

add_subdirectory(SGE)

option(RTTR_EDITOR_ADMINMODE "In admin mode there are some key combos to open debugger, resource viewer and so on" OFF)
if(RTTR_EDITOR_ADMINMODE)
	add_definitions(-D_ADMINMODE)
endif()

file(GLOB MAIN_SOURCES *.cpp *.h)
file(GLOB CIO_SOURCES CIO/*.cpp CIO/*.h)
SOURCE_GROUP(Main FILES ${MAIN_SOURCES})
SOURCE_GROUP(CIO FILES ${CIO_SOURCES})

if(WIN32)
    if(CMAKE_HOST_WIN32)
        SET(icon_RC "${CMAKE_CURRENT_SOURCE_DIR}/s25edit.rc")
        if(MSVC)
            ADD_FLAGS(CMAKE_EXE_LINKER_FLAGS /MANIFEST:NO)
        endif()
    else()
        ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/s25edit.res.o
                           COMMAND ${CMAKE_RC_COMPILER} -I${CMAKE_CURRENT_SOURCE_DIR}/ -i${CMAKE_CURRENT_SOURCE_DIR}/s25edit.rc -o ${CMAKE_CURRENT_BINARY_DIR}/s25edit.res.o)

        SET(icon_RC ${CMAKE_CURRENT_BINARY_DIR}/s25edit.res.o)
    endif()
ELSE()
	SET(icon_RC "")
ENDIF()

add_executable(s25edit ${MAIN_SOURCES} ${CIO_SOURCES} ${icon_RC})
target_link_libraries(s25edit PRIVATE SGE rttrConfig s25Common gamedata endian::static Boost::nowide PUBLIC Boost::disable_autolinking)
target_include_directories(s25edit PRIVATE include)

if(MINGW)
  target_link_libraries(s25edit PRIVATE -mconsole)
endif()

if(RTTR_BINDIR)
	INSTALL(TARGETS s25edit RUNTIME DESTINATION ${RTTR_BINDIR})
endif()

if(ClangFormat_FOUND)
    add_clangFormat_files(${MAIN_SOURCES} ${CIO_SOURCES})
endif()

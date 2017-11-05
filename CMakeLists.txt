CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
IF(NOT CMAKE_VERSION VERSION_LESS 3.1)
    # Simplified variable expansion. No change for usual variable references so use faster one
	CMAKE_POLICY(SET CMP0053 NEW)
ENDIF()

project(s25edit)

OPTION(RTTR_ENABLE_WERROR "Build with warnings turned into errors" ON)

IF(MSVC)
	IF(RTTR_ENABLE_WERROR)
		add_compile_options(/WX)
	ENDIF()
    add_compile_options(/MP) # parallel compilation
    # disable MSVC posix functions
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_DEPRECATE)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
	ADD_DEFINITIONS(/wd"4250")
else()
    add_compile_options(-Wall)
	IF(RTTR_ENABLE_WERROR)
		add_compile_options(-Werror)
	ENDIF()
ENDIF()

if(WIN32)
    ADD_DEFINITIONS(-DNOMINMAX)
endif()

add_subdirectory(SGE)

option(RTTR_EDITOR_EDITORMODE "In editor mode it is possible to load, edit and save created maps" ON)
option(RTTR_EDITOR_ADMINMODE "In admin mode there are some key combos to open debugger, resource viewer and so on" OFF)
if(RTTR_EDITOR_EDITORMODE)
	add_definitions(-D_EDITORMODE)
endif()
if(RTTR_EDITOR_ADMINMODE)
	add_definitions(-D_ADMINMODE)
endif()

FIND_PACKAGE(SDL REQUIRED)
include_directories(SYSTEM ${SDL_INCLUDE_DIR})

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

target_link_libraries(s25edit SGE ${SDL_LIBRARY})

if(ClangFormat_FOUND)
    add_clangFormat_files(${MAIN_SOURCES} ${CIO_SOURCES})
endif()

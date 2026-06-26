# Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
# Copyright (C) 2009 - 2025 Settlers Freaks <sf-team at siedler25.org>
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16..3.20)

project(s25edit)

find_package(Boost 1.64 REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../libutil/cmake")

# glad: OpenGL loader (reused from s25client)
if(NOT TARGET glad)
    set(RTTR_OGL_MAJOR 2)
    set(RTTR_OGL_MINOR 0)
    set(RTTR_OGL_ES 0)
    set(RTTR_OGL_COMPAT 1)
    set(RTTR_OGL_GL4ES 0)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../glad/openglCfg.hpp.cmake ${CMAKE_CURRENT_BINARY_DIR}/include/openglCfg.hpp @ONLY)

    set(GLAD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../glad/OpenGL2.0Compat/src/glad.c)
    set(GLAD_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../glad/OpenGL2.0Compat/include)
    add_library(glad STATIC ${GLAD_SRC})
    target_include_directories(glad PUBLIC ${GLAD_INCLUDE} ${CMAKE_CURRENT_BINARY_DIR}/include)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_libraries(glad PUBLIC ${CMAKE_DL_LIBS})
    endif()
endif()

add_subdirectory(SGE)

option(RTTR_EDITOR_ADMINMODE "In admin mode there are some key combos to open debugger, resource viewer and so on" OFF)
if(RTTR_EDITOR_ADMINMODE)
    add_definitions(-D_ADMINMODE)
endif()

file(GLOB MAIN_SOURCES *.cpp *.h include/*.h)
file(GLOB CIO_SOURCES CIO/*.cpp CIO/*.h)
SOURCE_GROUP(Main FILES ${MAIN_SOURCES})
SOURCE_GROUP(CIO FILES ${CIO_SOURCES})

if(WIN32)
    if(CMAKE_HOST_WIN32)
        SET(icon_RC "${CMAKE_CURRENT_SOURCE_DIR}/s25edit.rc")
        if(MSVC)
            string(APPEND CMAKE_EXE_LINKER_FLAGS " /MANIFEST:NO")
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
target_link_libraries(s25edit PRIVATE SGE rttrConfig s25Common gamedata siedler2 endian::static glad Boost::nowide PUBLIC Boost::disable_autolinking Boost::program_options)
target_include_directories(s25edit PRIVATE include)
target_compile_features(s25edit PRIVATE cxx_std_17)

if(MINGW)
    target_link_libraries(s25edit PRIVATE -mconsole)
endif()

if(RTTR_BINDIR)
    INSTALL(TARGETS s25edit RUNTIME DESTINATION ${RTTR_BINDIR})
endif()

if(ClangFormat_FOUND)
    add_clangFormat_files(${MAIN_SOURCES} ${CIO_SOURCES})
endif()

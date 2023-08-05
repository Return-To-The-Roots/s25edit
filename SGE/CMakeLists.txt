# Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
# Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
#
# SPDX-License-Identifier: GPL-3.0-or-later

FIND_PACKAGE(SDL2 REQUIRED)

file(GLOB_RECURSE SGE_SOURCES *.cpp *.h)
add_library(SGE STATIC ${SGE_SOURCES})
target_link_libraries(SGE PUBLIC Boost::boost SDL2::SDL2)
target_include_directories(SGE PRIVATE include/SGE PUBLIC include)
target_compile_features(SGE PUBLIC cxx_std_17)
set_target_properties(SGE PROPERTIES CXX_EXTENSIONS OFF)

include(EnableWarnings)
enable_warnings(SGE)

if(ClangFormat_FOUND)
    add_clangFormat_files(${SGE_SOURCES})
endif()

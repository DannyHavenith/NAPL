
##          Copyright Danny Havenith 2010.
## Distributed under the Boost Software License, Version 1.0.
##    (See accompanying file LICENSE_1_0.txt or copy at
##          http://www.boost.org/LICENSE_1_0.txt)

## This is the main CMakeLists file for the napl project.


cmake_minimum_required(VERSION 3.28)

project(napl)
add_definitions(-D_SCL_SECURE_NO_WARNINGS)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(
    project_options
    INTERFACE
        cxx_std_17)

SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE( Boost CONFIG)

IF(${Boost_MINOR_VERSION} GREATER 34 AND NOT WIN32)
	FIND_PACKAGE( Boost CONFIG COMPONENTS python serialization system filesystem)
ENDIF (${Boost_MINOR_VERSION} GREATER 34 AND NOT WIN32)

include_directories( ${napl_SOURCE_DIR}/include)
include_directories( ${napl_SOURCE_DIR}/dannylib)
add_subdirectory (libs)
add_subdirectory (applications)

set(CPACK_PACKAGE_NAME "napl")
set(CPACK_PACKAGE_VENDOR "havenith.net")
set(CPACK_PACKAGE_CONTACT "dh.napl@havenith.net")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "napl, audio programming library")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "napl")
set(CPACK_PACKAGE_EXECUTABLES "naplui;napl executables")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE_1_0.txt")
include(InstallRequiredSystemLibraries)
include(CPack)

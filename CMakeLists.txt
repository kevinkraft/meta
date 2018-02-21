cmake_minimum_required(VERSION 3.9.2)
project(meta)

option(ENABLE_LIBCXX "Use libc++ for the C++ standard library (only for clang)" OFF)
option(ENABLE_PROFILING "Link against gperftools profiler library" OFF)
option(ENABLE_JEMALLOC "Link against jemalloc if available" ON)

##-------------------------------------------------------------------------------------------
##copied from github.com/meta-toolkit/meta/blob/develop/.appveyor.yml#L35-L46
option(USE_MSVC "" ON) # this probably does nothing
#option(ZLIB_URL "" http://zlib.net/zlib-1.2.11.tar.xz)
#option(ICU_URL "" http://download.icu-project.org/files/icu4c/60.2/icu4c-60_2-Win64-MSVC2015.zip)
## Build ZLIB from scratch
#IF(USE_MSVC) appveyor DownloadFile %ZLIB_URL% -FileName zlib.tar.xz
#IF(USE_MSVC) 7z x zlib.tar.xz -oC:\projects
#IF(USE_MSVC) 7z x zlib.tar -oC:\projects
#IF(USE_MSVC) move zlib-* zlib
#IF(USE_MSVC) cd zlib && mkdir build && cd build
#IF(USE_MSVC) cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=C:\projects\zlib
#IF(USE_MSVC) ninja install
## Use pre-built ICU libs
#IF(USE_MSVC) cd C:\projects
#IF(USE_MSVC) appveyor DownloadFile %ICU_URL% -FileName icu.zip
#IF(USE_MSVC) 7z x icu.zip -oC:\projects\icu
##-------------------------------------------------------------------------------------------

MESSAGE( STATUS "ENABLE_JEMALLOC: " ${ENABLE_JEMALLOC} ) #ON, its a memory allocation library
MESSAGE( STATUS "MINGW: " ${MINGW} ) #none
MESSAGE( STATUS "USE_MSVC: " ${USE_MSVC} ) #none

if (MINGW)
  # jemalloc is breaking things in MSYS2; just disable it for now
  set(ENABLE_JEMALLOC OFF)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(MeTA_VERSION_MAJOR 3)
MESSAGE( STATUS "MeTA_VERSION_MAJOR: " ${MeTA_VERSION_MAJOR} ) #3
set(MeTA_VERSION_MINOR 0)
set(MeTA_VERSION_PATCH 2)
set(MeTA_VERSION
    "${MeTA_VERSION_MAJOR}.${MeTA_VERSION_MINOR}.${MeTA_VERSION_PATCH}")

MESSAGE( STATUS "MSVC_VERSION: " ${MSVC_VERSION} ) #1912
MESSAGE( STATUS "CMAKE_COMPILER_ID: " ${CMAKE_COMPILER_ID} ) #none
if (NOT (MSVC_VERSION AND CMAKE_COMPILER_ID MATCHES "Clang"))
  # MeTA only requires C++11, but on all of our supported compilers at least
  # C++1y support exists, so we therefore set the standard to 14
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()
MESSAGE( STATUS "CMAKE_CXX_STANDARD: " ${CMAKE_CXX_STANDARD} ) #14

MESSAGE( STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR} ) #D:/Programs/meta-toolkit
set(META_PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR})

include(CMakePushCheckState) #run cmake from this file, this file is cmake built in. clears other state vars?
include(ExternalProject) #I guess this is importing functions to use
include(deps/meta-cmake/FindOrBuildICU.cmake) #seems to import a function for finding or downloading ICU, called later
include(deps/meta-cmake/SetClangOptions.cmake) #clang only
include(deps/meta-cmake/CompilerKludges.cmake) #workaraounds and compiler fixes

find_package(Threads REQUIRED)
message("Threads_FOUND: " ${Threads_FOUND}) #true
find_package(ZLIB REQUIRED)
message("ZLIB_FOUND: " ${ZLIB_FOUND}) #true
find_package(LibLZMA)
message("LibLZMA_FOUND: " ${LibLZMA_FOUND}) #false, for compression, probably not necessary to have this and ZLIB

if (LIBLZMA_FOUND AND LIBLZMA_HAS_EASY_ENCODER)
  set(META_HAS_LIBLZMA ON)
endif()

cmake_push_check_state() #something to do with saving the config already done so it doesn't have t be done again

# Work around CMake not propagating the standard flag down to the compiler
message("CMAKE_REQUIRED_FLAGS: " ${CMAKE_REQUIRED_FLAGS}) #none
message("CMAKE_CXX14_STANDARD_COMPILE_OPTION: " ${CMAKE_CXX14_STANDARD_COMPILE_OPTION}) #-std:c++14
if (NOT MSVC_VERSION)
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${CMAKE_CXX14_STANDARD_COMPILE_OPTION}")
endif()

# Check if there is no build type set. If meta itself is the root project,
# compile it in release mode instead. If we aren't the root project, just
# continue along with whatever we would do ordinarily (they *really* should
# be specifying a build type, but...)
message("CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
message("CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR})
message("CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR})
if (NOT CMAKE_BUILD_TYPE AND CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  message("-- No build type selected, defaulting to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

message("CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/findicu)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/meta-cmake/)
message("CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})

FindOrBuildICU(
  VERSION 60.2
  URL http://download.icu-project.org/files/icu4c/60.2/icu4c-60_2-src.tgz
  URL_HASH MD5=43861b127744b3c0b9d7f386f4b9fa40
)

message("BUILD_SHARED_LIBS: " ${BUILD_SHARED_LIBS}) #none by default, can set in CMakeSettings.json
message("$BUILD_INTERFACE: " ${BUILD_INTERFACE}) #none
message("INSTALL_INTERFACE: " ${INSTALL_INTERFACE})
add_library(meta-definitions INTERFACE) #A shared library is never made for this library
target_include_directories(meta-definitions INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> #will be equal to the right part if BUILD_INTERFACE is equal to the right part, otherwise nothing, isn't evaulated until later so can change
  $<INSTALL_INTERFACE:include>) #this function says add these header directories to the search path of this library. INTERFACE seems to mean something public. INTERFACE flags are only applied to its dependents,
  #PUBLIC has flags applied to itself and to dependents, PRIVATE only has flags applied to itself. 

if(UNIX OR MINGW)
  target_compile_options(meta-definitions INTERFACE -Wall -Wextra -pedantic)

  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    SetClangOptions(meta-definitions)
  endif()
endif()

message("ZLIB_INCLUDE_DIRS: " ${ZLIB_INCLUDE_DIRS}) #path goes to my anaconda dir
target_include_directories(meta-definitions SYSTEM INTERFACE ${ZLIB_INCLUDE_DIRS}) #SYSTEM means they are system (external?) packages.

if (META_HAS_LIBLZMA)
  target_compile_definitions(meta-definitions INTERFACE -DMETA_HAS_LIBLZMA=1)
endif()

message("LIBDL_LIBRARY: " ${LIBDL_LIBRARY}) #none, its a dynamic linking library, unix specific. Windows has its own Windows API
if (LIBDL_LIBRARY)
  target_link_libraries(meta-definitions INTERFACE ${LIBDL_LIBRARY})
endif()

message("CXXABI_LIBRARY: " ${CXXABI_LIBRARY}) #none, something to do with std C++?
if (CXXABI_LIBRARY)
  target_link_libraries(meta-definitions INTERFACE ${CXXABI_LIBRARY})
endif()

message("LIBCXX_FOUND: " ${LIBCXX_FOUND}) #none, this is clang specific
if (LIBCXX_FOUND)
  target_include_directories(meta-definitions SYSTEM INTERFACE ${LIBCXX_INCLUDE_DIR})
  target_compile_options(meta-definitions INTERFACE ${LIBCXX_OPTIONS})
  target_link_libraries(meta-definitions INTERFACE -L${LIBCXX_LIB_PATH})
  target_link_libraries(meta-definitions INTERFACE ${LIBCXX_LIBRARY})
endif()

message("ENABLE_PROFILING: " ${ENABLE_PROFILING}) #OFF
if (ENABLE_PROFILING)
  find_library(GPERFTOOLS_PROFILER NAMES profiler REQUIRED)
  message("-- Found profiler: ${GPERFTOOLS_PROFILER}")
  target_link_libraries(meta-definitions INTERFACE ${GPERFTOOLS_PROFILER})
endif()

find_library(JEMALLOC_LIB NAMES jemalloc) #JEMALLOC_LIB_NOTFOUND
message("JEMALLOC_LIB: " ${JEMALLOC_LIB})
if (JEMALLOC_LIB AND ENABLE_JEMALLOC)
  message("-- Using jemalloc: ${JEMALLOC_LIB}")
  target_link_libraries(meta-definitions INTERFACE ${JEMALLOC_LIB})
elseif(NOT MINGW)
  message("-- Using regular malloc; consider installing jemalloc")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin") #this is checking if the system is macOS.
  set(_DARWIN_USE_64_BIT_INODE 1)
  set(META_IS_DARWIN 1)
  target_compile_definitions(meta-definitions INTERFACE
                             -D_DARWIN_USE_64_BIT_INODE=1)
  target_compile_definitions(meta-definitions INTERFACE
                             -DMETA_IS_DARWIN=1)
endif()

# set a bunch of preprocessor variables to work around various compiler and
# standard library bugs
CompilerKludges(meta/kludges.h) #simply includes all the make files in kludges. This will be vary useful for defining platform specifics easily, just add a kludge.
#I don't know what or where this file arg is. Oh, something gets copied to this location, relative to build dir.

configure_file(include/meta/config.h.in meta/config.h) #copy file and modify contents, relative to build dir
target_include_directories(meta-definitions INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>)

cmake_pop_check_state()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_custom_target(tidy
                  COMMAND /usr/bin/rm -rf
                  ./doc
                  *.terms
                  *.phi
                  *.theta)

find_package(Doxygen)
if(DOXYGEN_FOUND AND NOT TARGET doc)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/meta.doxygen.in
                 ${CMAKE_CURRENT_BINARY_DIR}/meta.doxygen @ONLY)
  add_custom_target(doc
                    ${DOXYGEN_EXECUTABLE}
                    ${CMAKE_CURRENT_BINARY_DIR}/meta.doxygen
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(deps/cpptoml EXCLUDE_FROM_ALL)

# Warn users that are using a 32-bit system
if (CMAKE_SIZEOF_VOID_P LESS 8)
  message(WARNING "You appear to be running on a 32-bit system. Support \
    for 32-bit systems is provided on a best-effort basis; if at all \
    possible, we strongly recommend that you use  MeTA on a 64-bit \
    platform.")
endif()

# install our targets defined in this file
install(TARGETS meta-definitions
        EXPORT meta-exports
        DESTINATION lib)

# install all of our includes
install(DIRECTORY include/meta/
        DESTINATION include/meta
        FILES_MATCHING PATTERN "*.h"
                       PATTERN "*.tcc")
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/meta/
        DESTINATION include/meta
        FILES_MATCHING PATTERN "*.h")

# generate our ConfigVersion.cmake file
configure_file(cmake/MeTAConfigVersion.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/MeTA/MeTAConfigVersion.cmake
               @ONLY)
configure_file(cmake/MeTAConfig.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/MeTA/MeTAConfig.cmake
               COPYONLY)

# install our exports and cmake files
install(EXPORT meta-exports
        FILE MeTATargets.cmake
        DESTINATION lib/cmake/MeTA)
install(FILES
          ${CMAKE_CURRENT_BINARY_DIR}/MeTA/MeTAConfigVersion.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/MeTA/MeTAConfig.cmake
        DESTINATION
          lib/cmake/MeTA)

# allow consumption of a build directory as an "installed" version
export(EXPORT meta-exports
       FILE ${CMAKE_CURRENT_BINARY_DIR}/MeTA/MeTATargets.cmake)
export(PACKAGE MeTA)

cmake_minimum_required(VERSION 3.4.0)

project(pxsle)
add_definitions(-DPACKAGE_VERSION="prototype")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/macros)

include_directories("include")

# Dependencies.
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})
find_library(DL_LIB dl REQUIRED)
find_library(RT_LIB rt REQUIRED)
find_package(X11 REQUIRED)
include_directories(${X11_XTest_INCLUDE_PATH})
include(GlibCompileResourcesSupport)
find_package(GLib REQUIRED)
include_directories(${GLib_INCLUDE_DIRS})

set(SRCS
 src/main.cc
)
add_executable(psxle ${SRCS} )

target_link_libraries(psxle pcsxcore ${GTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ZLIB_LIBRARIES} ${RT_LIB} ${DL_LIB} ${X11_XTest_LIB})

# Components.
add_subdirectory(libpcsxcore)
add_subdirectory(plugins)

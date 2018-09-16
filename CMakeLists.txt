cmake_minimum_required(VERSION 3.4.0)

project(pxsle)
add_definitions(-DPACKAGE_VERSION="prototype")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/macros)

include(GlibCompileResourcesSupport)

include_directories("include")

# Dependencies.
find_package(GLib REQUIRED)
if(NOT GLib_FOUND)
    message(FATAL_ERROR "GLIB2 library not found")
endif(NOT GLib_FOUND)
include_directories(${GLib_INCLUDE_DIRS})

find_package(GTK3 3.14.0 REQUIRED)
find_package(GTK3 REQUIRED)
if(NOT GTK3_FOUND)
    message(FATAL_ERROR "GTK3 library not found")
endif(NOT GTK3_FOUND)
include_directories(${GTK3_INCLUDE_DIRS})
set(GTK_LIBRARIES ${GTK3_LIBRARY} ${GDK3_LIBRARY} ${GDKPixbuf_LIBRARY} ${Pango_LIBRARY} ${Cairo_LIBRARY} ${GObject_LIBRARY} ${GLib_LIBRARY} ${GIO_LIBRARY})

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

find_library(DL_LIB dl REQUIRED)
find_library(RT_LIB rt REQUIRED)

find_package(X11 REQUIRED)
include_directories(${X11_XTest_INCLUDE_PATH})

set(SRCS
 src/main.cc
)
add_executable(psxle ${SRCS} )

target_link_libraries(psxle pcsxcore ${CMAKE_THREAD_LIBS_INIT} ${ZLIB_LIBRARIES} ${RT_LIB} ${DL_LIB} ${X11_XTest_LIB})

# Components.
add_subdirectory(libpcsxcore)
add_subdirectory(plugins)

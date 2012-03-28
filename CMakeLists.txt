cmake_minimum_required (VERSION 2.6)
project (Psychopath)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pg")

set(CMAKE_BUILD_TYPE Release)

set(OIIO_DIR ${PROJECT_SOURCE_DIR}/../libs/oiio/dist/linux64)
set(OIIO_LIBRARY ${OIIO_DIR}/lib/libOpenImageIO.so)
set(OIIO_INCLUDE ${OIIO_DIR}/include)

include_directories ("${PROJECT_SOURCE_DIR}",
                     "${PROJECT_SOURCE_DIR}/acceleration",
                     "${PROJECT_SOURCE_DIR}/basics",
                     "${PROJECT_SOURCE_DIR}/grid",
                     "${PROJECT_SOURCE_DIR}/math",
                     "${PROJECT_SOURCE_DIR}/primitives",
                     "${PROJECT_SOURCE_DIR}/sampling",
                     "${OIIO_INCLUDE}")

add_subdirectory(acceleration)
add_subdirectory(basics)
add_subdirectory(grid)
add_subdirectory(math)
add_subdirectory(primitives)
add_subdirectory(sampling)

add_executable(psychopath
    config main raster)

target_link_libraries(psychopath
    sampling primitives basics grid acceleration ${OIIO_LIBRARY})
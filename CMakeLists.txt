cmake_minimum_required(VERSION 3.10)

project(baji C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set( CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused -g" )
set( CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused -g" )

add_definitions(-DSIMULATOR=1)
add_definitions(-DCONFIG_BUILD_VERSION=\"1.0.0\")

add_subdirectory(transformers)
find_package(SDL2 REQUIRED SDL2)
find_package(Freetype REQUIRED)
include_directories(
    ${AI_S2_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}/../
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/lvgl
)

FILE(GLOB_RECURSE LVGL_Sources CONFIGURE_DEPENDS lvgl/*.c)


add_executable(${PROJECT_NAME} main.c lv_port_fs.c ${LVGL_Sources} ${UI_Sources} ${AI_S2_SOURCES})

#string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)

target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES}  ${FREETYPE_LIBRARIES} m)
target_link_options(baji PRIVATE
    -Wl,--undefined=__start_cl_ui_page
    -Wl,--undefined=__stop_cl_ui_page)
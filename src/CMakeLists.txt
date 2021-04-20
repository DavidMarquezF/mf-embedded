cmake_minimum_required(VERSION 3.5)


list(APPEND EXTRA_COMPONENT_DIRS "/home/david/iot-lite/iotivity-lite/port/esp32/builder")

set(PROJECT_VER "0.0.0.0")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Compile definitions must be set before the project command in order to make them available throughout the whole project
add_definitions(-DMF_NUM_COMPONENTS=3)
add_definitions(-DOC_CLOUD -DOC_CLIENT -DOC_SERVER -DOC_DYNAMIC_ALLOCATION -DOC_TCP -DOC_IPV4) #TODO: Find a way of deducing this like in the main CMake (from the kconfig)


project(example)

idf_build_get_property(config_dir CONFIG_DIR)

set(sdkconfig_header ${config_dir}/sdkconfig.h)
#set(ENV{MF_COMPONENTS_INCLUDE} src/components/include)

set(project_elf ${CMAKE_PROJECT_NAME}.elf)

message("CONFIGURATION")
message("CONFIG DIR: ${CONFIG_DIR}")
message( "COMPONENTS DIRS: ${COMPONENT_DIRS}" )
message( "EXTRA COMPONENTS: ${EXTRA_COMPONENT_DIRS}" )



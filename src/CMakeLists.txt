cmake_minimum_required(VERSION 3.5)


list(APPEND EXTRA_COMPONENT_DIRS "/home/david/iot-lite/iotivity-lite/port/esp32/builder")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(example)

idf_build_get_property(config_dir CONFIG_DIR)

set(ENV{MF_COMPONENTS_INCLUDE} src/components/include)

set(sdkconfig_header ${config_dir}/sdkconfig.h)
set(project_elf ${CMAKE_PROJECT_NAME}.elf)

message("CONFIGURATION")
message("CONFIG DIR: ${CONFIG_DIR}")
message( "COMPONENTS DIRS: ${COMPONENT_DIRS}" )
message( "EXTRA COMPONENTS: ${EXTRA_COMPONENT_DIRS}" )



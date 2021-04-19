if (CONFIG_OC_DEBUG)
  add_definitions(-DOC_DEBUG)
endif()

if (CONFIG_IPV4)
  add_definitions(-DOC_IPV4)
endif()

if (CONFIG_TCP)
  add_definitions(-DOC_TCP)
endif()

if (CONFIG_CLOUD)
  add_definitions(-DOC_CLOUD -DOC_DYNAMIC_ALLOCATION -DOC_TCP -DOC_IPV4)
endif()

if (CONFIG_DYNAMIC)
	add_definitions(-DOC_DYNAMIC_ALLOCATION)
endif()

if (CONFIG_SECURE)
	add_definitions(-DOC_SECURITY -DOC_PKI -DAPP_DEBUG)
endif()

#add_definitions(-DOC_CLIENT)
#add_definitions(-DOC_SERVER)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format-truncation")

idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS ""
                    PRIV_REQUIRES iotivity nvs_flash mf_spi mf_spi_device mf_main mf_updates_handler)

set_source_files_properties("main.c"
   PROPERTIES COMPILE_FLAGS
    "-DOC_SERVER -DOC_CLIENT"
)


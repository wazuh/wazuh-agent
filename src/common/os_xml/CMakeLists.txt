file(GLOB OS_XML_SOURCES src/*.c)

add_library(os_xml STATIC ${OS_XML_SOURCES})

target_include_directories(os_xml PUBLIC include)

target_link_libraries(os_xml utils file_op)
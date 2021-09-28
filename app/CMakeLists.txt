cmake_minimum_required (VERSION 3.14)

set(targetname interface_generator_app)

file(GLOB SRCFILE *.h *.cpp)

add_executable(${targetname} ${SRCFILE})


target_include_directories(${targetname} PRIVATE ../include)

target_link_libraries(${targetname} PRIVATE abi_interface_generator)

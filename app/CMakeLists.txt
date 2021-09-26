cmake_minimum_required (VERSION 3.14)

set(targetname interface_generator_app)

file(GLOB SRCFILE *.h *.cpp)

add_executable(${targetname} ${SRCFILE})


target_include_directories(${targetname} PRIVATE ../abi_interface_generator)

# link
message("CMAKE_BUILD_TYPE:${CMAKE_BUILD_TYPE}")
if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
	set(LIB_DIR "../debug/lib")
else()
	set(LIB_DIR)
endif()
message("LIB_DIR:${LIB_DIR}")

#target_link_directories(${targetname} PRIVATE ${LLVM_LIB_DIR})

target_link_libraries(${targetname} PRIVATE abi_interface_generator)

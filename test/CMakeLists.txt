cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(WIN32)
	find_package(GTest CONFIG REQUIRED)
endif()

file(GLOB test_srcs *.h *.hpp *.cpp)

set(target_name  ${PROJECT_NAME}_test)

add_executable(${target_name} ${test_srcs})

target_include_directories(${target_name} PRIVATE ../include)


target_link_directories(${target_name} PRIVATE ./lib)


if(WIN32)
	target_link_libraries(${target_name} PRIVATE GTest::gtest)
else()
	target_link_libraries(${target_name} PRIVATE gtest)
endif()
target_link_libraries(${target_name} PRIVATE abi_interface_generator)


enable_testing()
add_test(NAME ${target_name} COMMAND ${target_name})

cmake_minimum_required (VERSION 3.14)

#set(CMAKE_C_STANDARD 17) 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project ("interface_generator")

if((NOT (CMAKE_BUILD_TYPE STREQUAL "Debug")) AND (NOT (CMAKE_BUILD_TYPE STREQUAL "Release")))
	set(CMAKE_BUILD_TYPE "Release")
endif()

find_package(LLVM CONFIG REQUIRED)
find_package(inja CONFIG REQUIRED)

add_subdirectory("src")
add_subdirectory("app")

enable_testing()
add_subdirectory("test")

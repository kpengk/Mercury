cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DINTERFACE_GENERATOR_STATIC)

if(WIN32)
	find_package(GTest CONFIG REQUIRED)
endif()

file(GLOB test_srcs *.cpp ../src/*.cpp)

set(target_name  ${PROJECT_NAME}_test)

add_executable(${target_name} ${test_srcs})

target_include_directories(${target_name} PRIVATE ../include)


# link
message("CMAKE_BUILD_TYPE:${CMAKE_BUILD_TYPE}")
if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
	set(LLVM_LIB_DIR "${LLVM_LIBRARY_DIRS}/../debug/lib")
else()
	set(LLVM_LIB_DIR ${LLVM_LIBRARY_DIRS})
endif()
message("LLVM_LIB_DIR:${LLVM_LIB_DIR}")

target_link_directories(${target_name} PRIVATE ${LLVM_LIB_DIR})

file(GLOB LIBS ${LLVM_LIB_DIR}/*.lib)

if(WIN32)
	target_link_libraries(${target_name} PRIVATE ${LIBS} pantor::inja GTest::gtest)
else()
	target_link_libraries(${target_name} PRIVATE ${LIBS} pantor::inja gtest)
endif()


enable_testing()
add_test(NAME ${target_name} COMMAND ${target_name})

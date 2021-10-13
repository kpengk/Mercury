cmake_minimum_required (VERSION 3.14)

add_definitions(-DINTERFACE_GENERATOR_EXPORT)

set(targetname ${PROJECT_NAME})

file(GLOB SRCFILE *.h *.cpp)

add_library(${targetname} SHARED ${SRCFILE})


target_include_directories(${targetname} PRIVATE ${LLVM_INCLUDE_DIRS} ../include)

# link
message("CMAKE_BUILD_TYPE:${CMAKE_BUILD_TYPE}")
if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
	set(LLVM_LIB_DIR "${LLVM_LIBRARY_DIRS}/../debug/lib")
else()
	set(LLVM_LIB_DIR ${LLVM_LIBRARY_DIRS})
endif()
message("LLVM_LIB_DIR:${LLVM_LIB_DIR}")

target_link_directories(${targetname} PRIVATE ${LLVM_LIB_DIR})

file(GLOB LIBS ${LLVM_LIB_DIR}/*.lib)

target_link_libraries(${targetname} PRIVATE ${LIBS} pantor::inja)

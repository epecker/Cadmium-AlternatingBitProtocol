cmake_minimum_required (VERSION 3.20)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_COMPILER "g++")
project(ABP)
add_compile_options(-g)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../cadmium/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../DESTimes/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../CadmiumModelJSONExporter/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(
	Boost 1.40 REQUIRED
	COMPONENTS program_options 
)

file(MAKE_DIRECTORY simulation_results)

set(SOURCES_LIB "data_structures/message.cpp" "data_structures/message.hpp")
add_library(messages STATIC ${SOURCES_LIB})

add_executable(test_receiver test/main_receiver_test.cpp)
add_executable(test_sender test/main_sender_test.cpp)
add_executable(test_subnet test/main_subnet_test.cpp)
add_executable(simulator top_model/main.cpp)

target_link_libraries(test_receiver Boost::program_options messages)
target_link_libraries(test_sender Boost::program_options messages)
target_link_libraries(test_subnet Boost::program_options messages)
target_link_libraries(simulator Boost::program_options messages)

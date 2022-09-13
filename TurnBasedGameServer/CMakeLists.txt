cmake_minimum_required (VERSION 3.8)
include(CMakePrintHelpers)


set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE MATCHES Release)
    set(SFML_STATIC_LIBRARIES TRUE)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS On)

find_package(SFML COMPONENTS system network graphics CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

include_directories(include/)

# Add source to this project's executable.  
file(GLOB_RECURSE PROJECT_SRC src/*.cpp include/*.h[pp])

set (SRC ${PROJECT_SRC} ${PROJECT_COMMON})
cmake_print_variables(SRC)

add_executable(${EXECUTABLE_CUSTOM_NAME} "${SRC}")

# Link libraries
target_link_libraries(${EXECUTABLE_CUSTOM_NAME} PRIVATE sfml-network sfml-system sfml-graphics)
target_link_libraries(${EXECUTABLE_CUSTOM_NAME} PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

cmake_minimum_required (VERSION 3.8)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE MATCHES Release)
    set(SFML_STATIC_LIBRARIES TRUE)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /w14640)
    add_compile_definitions(_USE_MATH_DEFINES)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS On)

# Add data files
file(GLOB_RECURSE DATA_FILES data/*)
source_group("data/" FILES ${DATA_FILES})
foreach(DATA ${DATA_FILES})
    get_filename_component(FILE_NAME ${DATA} NAME)
    get_filename_component(PATH_NAME ${DATA} DIRECTORY)
    get_filename_component(EXTENSION ${DATA} EXT)
    file(RELATIVE_PATH PATH_NAME "${PROJECT_SOURCE_DIR}" ${PATH_NAME})
    set(DATA_OUTPUT "${PROJECT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
    add_custom_command(
            OUTPUT ${DATA_OUTPUT}
            DEPENDS ${DATA}
            DEPENDS
            COMMAND ${CMAKE_COMMAND} -E copy ${DATA} "${PROJECT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}"
    )
    list(APPEND DATA_BINARY_FILES ${DATA_OUTPUT})
endforeach()

add_custom_target(
        DataTarget-${EXECUTABLE_CUSTOM_NAME}
        DEPENDS ${DATA_BINARY_FILES} ${DATA_FILES})

# Find vcpkg libraries
find_package(OpenGL REQUIRED)
find_package(SFML COMPONENTS system window graphics network main audio CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)

include_directories(include/)

# Add source to this project's executable.  
file(GLOB_RECURSE PROJECT_SRC src/*.cpp include/*.h[pp])

set (SRC ${PROJECT_SRC} ${PROJECT_COMMON})

add_executable(${EXECUTABLE_CUSTOM_NAME} "${SRC}")
# add_executable(${EXECUTABLE_CUSTOM_NAME} WIN32 "${SRC}")

# Add data dependencies
add_dependencies(${EXECUTABLE_CUSTOM_NAME} DataTarget-${EXECUTABLE_CUSTOM_NAME})

# Link libraries
target_link_libraries(${EXECUTABLE_CUSTOM_NAME} PRIVATE sfml-graphics sfml-audio sfml-window sfml-system sfml-network)

if (WIN32)
target_link_libraries(${EXECUTABLE_CUSTOM_NAME} PRIVATE sfml-main)
endif()

if(MSVC)
target_link_libraries(${EXECUTABLE_CUSTOM_NAME} PRIVATE FLAC OpenAL OpenGL Vorbis)
else()
target_link_libraries(${EXECUTABLE_CUSTOM_NAME} PRIVATE X11 FLAC UDev OpenAL ${OPENGL_LIBRARIES} )
endif()

target_link_libraries(${EXECUTABLE_CUSTOM_NAME} PRIVATE imgui::imgui)
target_link_libraries(${EXECUTABLE_CUSTOM_NAME} PRIVATE ImGui-SFML::ImGui-SFML)

cmake_minimum_required (VERSION 3.8)

# Name of the solution
project ("TurnBasedGame")

if ( NOT CMAKE_BUILD_TYPE )
    message( "Build Type not set, defaulting to Debug..." )
    set( CMAKE_BUILD_TYPE Debug )
endif()

include_directories(common/include/)
file(GLOB_RECURSE PROJECT_COMMON common/src/*.cpp common/include/*.h[pp])

# Include sub-projects.
set(EXECUTABLE_CUSTOM_NAME "TurnBasedGame")
add_subdirectory ("TurnBasedGame")

set(EXECUTABLE_CUSTOM_NAME "TurnBasedGameServer")
add_subdirectory ("TurnBasedGameServer")

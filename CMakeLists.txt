cmake_minimum_required(VERSION 4.0)
project(Boids_simulation)

set(CMAKE_CXX_STANDARD 17)


#From the SFML site for CMake configurations
include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(Boids_simulation main.cpp)

target_compile_features(Boids_simulation PRIVATE cxx_std_17)
target_link_libraries(Boids_simulation PRIVATE SFML::Graphics)

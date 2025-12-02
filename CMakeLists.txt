cmake_minimum_required(VERSION 4.0)
project(Boids_simulation)

set(CMAKE_CXX_STANDARD 20)

#To use openmp
if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xpreprocessor -fopenmp -lomp")
endif ()

#To use SIMD instructions. Include -mavx.
add_compile_options("-mavx2")

#From the SFML site for CMake configurations
include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SFML)

#Differentiations depending on CMake profile
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options("-fsanitize=address,leak,undefined")
    add_link_options("-fsanitize=address,leak,undefined")
    add_compile_options("-g")

elseif (CMAKE_BUILD_TYPE STREQUAL "ThreadsDebugging") #separated, need different flags
    # works better disabling ASLR (for LINUX, in terminal: sudo echo 0 | sudo tee /proc/sys/kernel/randomize_va_space)
    # print 0 if ASLR has been disabled
    add_compile_options("-fsanitize=thread")
    add_compile_options("-g")
    add_compile_options("-O2")
    add_link_options("-fsanitize=thread")
elseif (CMAKE_BUILD_TYPE STREQUAL "Benchmark")
    add_compile_options("-O3")
    add_compile_options("-mfma")
    add_compile_options("-march=native")
    add_compile_options("-fvectorize")
    add_compile_options("-fno-math-errno")
    add_compile_options("-gdwarf-4")
    add_compile_options("-g")
endif()

add_executable(Boids_simulation main.cpp)

target_compile_features(Boids_simulation PRIVATE cxx_std_17)
target_link_libraries(Boids_simulation PRIVATE SFML::Graphics)

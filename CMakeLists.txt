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
add_compile_options("-mavx")
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

elseif (CMAKE_BUILD_TYPE STREQUAL "Benchmark")
    add_compile_options("-O3")
    add_compile_options("-mfma")
    add_compile_options("-march=native")
    add_compile_options("-fno-math-errno")
    add_compile_options("-ffast-math")
endif()


add_executable(AOS AOS.cpp AOS_helper.h)
target_compile_features(AOS  PRIVATE cxx_std_17)
target_link_libraries(AOS  PRIVATE SFML::Graphics)

add_executable(AOS_parallel_SIMD AOS_parallel_SIMD.cpp AOS_helper_SIMD.h)
target_compile_features(AOS_parallel_SIMD  PRIVATE cxx_std_17)
target_link_libraries(AOS_parallel_SIMD  PRIVATE SFML::Graphics)

add_executable(SOA SOA.cpp SOA_helper.h)
target_compile_features(SOA  PRIVATE cxx_std_17)
target_link_libraries(SOA  PRIVATE SFML::Graphics)

add_executable(SOA_parallel_SIMD SOA_parallel_SIMD.cpp SOA_helper_SIMD.h)
target_compile_features(SOA_parallel_SIMD  PRIVATE cxx_std_17)
target_link_libraries(SOA_parallel_SIMD  PRIVATE SFML::Graphics)



cmake_minimum_required(VERSION 3.22.1)
project(cle-weather-stations-mt CXX)

add_executable(cle-ws-mt
    src/main.cpp
    # Add your files here if needed.
)

target_include_directories(cle-ws-mt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(cle-ws-mt PRIVATE multithreaded)

target_compile_features(cle-ws-mt PRIVATE cxx_std_20)
target_compile_options(cle-ws-mt PRIVATE -Wall -Wextra)

cmake_minimum_required(VERSION 3.22.1)
project(cle-weather-stations CXX)

add_executable(cle-ws
    src/main.cpp
    # Add your files here if needed.
)
target_compile_features(cle-ws PRIVATE cxx_std_20)
target_compile_options(cle-ws PRIVATE -Wall -Wextra)

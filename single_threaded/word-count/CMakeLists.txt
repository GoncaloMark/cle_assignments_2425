cmake_minimum_required(VERSION 3.22.1)
project(cle-word-count CXX)

add_executable(cle-wc
    src/main.cpp
    src/utf-8.cpp
    src/word_count.cpp
    # Add your files here if needed.
)
target_compile_features(cle-wc PRIVATE cxx_std_20)
target_compile_options(cle-wc PRIVATE -Wall -Wextra)
target_include_directories(cle-wc PRIVATE src)

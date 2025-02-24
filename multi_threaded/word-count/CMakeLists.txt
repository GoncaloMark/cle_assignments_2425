cmake_minimum_required(VERSION 3.22.1)
project(cle-word-count-mt CXX)

add_executable(cle-wc-mt
    src/main.cpp
    src/utf-8.cpp
    src/word_count.cpp
    # Add your files here if needed.
)
target_compile_features(cle-wc-mt PRIVATE cxx_std_20)
target_compile_options(cle-wc-mt PRIVATE -Wall -Wextra)
target_include_directories(cle-wc-mt PRIVATE src)

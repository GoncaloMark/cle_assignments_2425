add_library(multithreaded STATIC threading.cpp)

target_include_directories(multithreaded PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(multithreaded PRIVATE cxx_std_20)
target_compile_options(multithreaded PRIVATE -Wall -Wextra)

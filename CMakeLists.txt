cmake_minimum_required(VERSION 3.22.1)  

project(assignment1 CXX)

add_subdirectory(multi_threaded/word-count)
add_subdirectory(single_threaded/word-count)
add_subdirectory(multi_threaded/weather-stations)
add_subdirectory(single_threaded/weather-stations)
add_subdirectory(samples)

add_subdirectory(multi_threaded/shared)
add_subdirectory(multi_threaded/tests)

enable_testing()

add_test(NAME queueTest COMMAND $<TARGET_FILE:queueTest>)


cmake_minimum_required( VERSION 2.8.9 )

project( mandlebrot )

find_library( PNG png )

find_library( Z z )

add_executable( mandelbrot mandelbrot.c )

target_link_libraries( mandelbrot png z m )


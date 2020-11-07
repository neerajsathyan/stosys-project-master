#
# MIT License
#
#Copyright (c) 2020-2021
#
#Authors: Sacheendra Talluri, Giulia Frascaria, and, Animesh Trivedi
#This code is part of the Storage System Course at VU Amsterdam
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all
#copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#SOFTWARE.
#

cmake_minimum_required(VERSION 3.10)
project(ocssd_kv)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

add_executable(milestone2 src/assignment2_test_without_gc.cpp src/ftl.h src/ftl.cpp)
add_executable(milestone2c src/assignment2_test_without_gc.cpp src/cftl.h src/cppwrapper.h src/cftl.c)
add_executable(milestone3 src/assignment3_test_with_gc.cpp src/ftl.h src/ftl.cpp)
add_executable(milestone3c src/assignment3_test_with_gc.cpp src/cftl.h src/cppwrapper.h src/cftl.c)
add_executable(kv src/kv.c src/kv.h src/cftl.h src/ftl.h src/ftl.cpp)

target_compile_definitions(milestone2 PUBLIC cppftl)
target_compile_definitions(milestone3 PUBLIC cppftl)

target_link_libraries(milestone2 lightnvm aio)
target_link_libraries(milestone2c lightnvm aio)
target_link_libraries(milestone3 lightnvm aio)
target_link_libraries(milestone3c lightnvm aio)
target_link_libraries(kv lightnvm aio)
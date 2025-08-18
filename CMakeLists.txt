cmake_minimum_required(VERSION 4.0)
project(avrIntepreter)

set(CMAKE_CXX_STANDARD 20)

add_executable(avrIntepreter main.cpp
        Eval.h
        Eval.cpp)

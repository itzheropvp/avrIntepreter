cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(avrInterpreter)

add_library(avrInterpreter
        src/Eval.cpp
        src/Interpreter.cpp
)

target_include_directories(avrInterpreter PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(avrInterpreter_cli src/main.cpp)
target_link_libraries(avrInterpreter_cli PRIVATE avrInterpreter)
cmake_minimum_required(VERSION 3.15)
project(avrInterpreter)

set(CMAKE_CXX_STANDARD 17)

add_library(avrInterpreter
        src/Eval.cpp
        src/Interpreter.cpp
)

target_include_directories(avrInterpreter PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(avrInterpreter_cli src/main.cpp)
target_link_libraries(avrInterpreter_cli PRIVATE avrInterpreter)

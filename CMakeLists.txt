cmake_minimum_required(VERSION 3.10.0)
project(PingPongOS VERSION 0.1.0 LANGUAGES C)

add_executable(testafila queue/testafila.c queue/queue.c)
target_include_directories(testafila PRIVATE ${CMAKE_SOURCE_DIR}/logger)

add_compile_options(-Og -std=c99 -Wall -Wextra -Wshadow -Wconversion -Wpedantic -Wsign-compare -Wconversion -Wunused -Wunused-parameter -Wunused-variable -Wunused-function -Wunused-but-set-variable -Wunused-but-set-parameter -Wunused-result)

add_compile_definitions(LOG LOG_LEVEL=${LOG_LEVEL})

add_custom_target(run-tests
    COMMAND ${CMAKE_COMMAND} --build . --target testafila
    COMMAND ./testafila
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS testafila
)
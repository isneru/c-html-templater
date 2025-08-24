cmake_minimum_required(VERSION 3.10)

project(c_html_templater)

set(CMAKE_C_STANDARD 11)

add_executable(${PROJECT_NAME} main.c server.c)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()
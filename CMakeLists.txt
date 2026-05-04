cmake_minimum_required(VERSION 3.10)
project(serveur_web)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(serveur_web ${SOURCES}
        include/server.h
        src/server.cpp
        include/request.h
        src/request.cpp
        include/response.h)

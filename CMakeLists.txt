cmake_minimum_required(VERSION 3.20)
project(ft_irc)

set(CMAKE_CXX_STANDARD 11)

include_directories(src/)

add_executable(ft_irc
        main.cpp
        src/Server.cpp
        src/User.cpp
        )

add_executable(showip
        showip.cpp)

add_executable(test
        test.cpp)
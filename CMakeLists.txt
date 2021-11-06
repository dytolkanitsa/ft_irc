cmake_minimum_required(VERSION 3.20)
project(ft_irc)

set(CMAKE_CXX_STANDARD 11)

include_directories(src/)

add_executable(ft_irc
        main.cpp
        src/Server.cpp
        src/User.cpp
        src/Channel.cpp
        src/Command.cpp
        src/Message.cpp
        src/OtherServerMethods.cpp
        )

add_executable(bot
        bot/Bot.cpp
        bot/Bot.hpp
        bot/mainBot.cpp
        src/User.cpp
        src/Channel.cpp
        )

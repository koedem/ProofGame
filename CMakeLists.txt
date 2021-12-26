cmake_minimum_required(VERSION 3.21)
project(ProofGame)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(Surge)

add_executable(ProofGame
        Surge/chess_engine.cpp
        forbiddenTT.h
        main.cpp
        perftTT8.h
        Surge/position.cpp
        Surge/position.h
        proof_game.h
        Surge/tables.cpp
        Surge/tables.h
        Surge/types.cpp
        Surge/types.h)

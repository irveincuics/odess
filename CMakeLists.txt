cmake_minimum_required(VERSION 3.27)
project(odesss)

set(CMAKE_CXX_STANDARD 17)

add_executable(odesss main.cpp
        RabinFingerprint.cpp
        N_Transform.cpp
        GearFingerprint.cpp
        Odess.cpp
)

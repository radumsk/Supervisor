cmake_minimum_required(VERSION 3.26)
project(SupervisorDaemon C)
project(SupervisorTest C)
project(SupervisorLibrary C)

set(CMAKE_C_STANDARD 23)

add_executable(SupervisorDaemon
        daemon/daemon.c
        daemon/listener.c
        shared/socket_encoding.h
        shared/socket_encoding.c
        daemon/methods.c
        daemon/methods.h)

add_library(SupervisorLibrary SHARED library/supervisor.c library/supervisor.h
        library/service.h
        library/service.c)

target_link_libraries(SupervisorLibrary pthread)

add_executable(SupervisorTest
        shared/socket_encoding.c
        shared/socket_encoding.h
        library/supervisor.h
        library/supervisor.c
        test/main.c)
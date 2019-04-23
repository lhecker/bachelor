cmake_minimum_required(VERSION 3.8)
project(bachelor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else ()
        set(CMAKE_CXX_FLAGS "/W4 ${CMAKE_CXX_FLAGS}")
    endif ()
else ()
    set(CMAKE_CXX_FLAGS "-Wall -Wextra ${CMAKE_CXX_FLAGS}")
endif ()

find_package(OpenCV REQUIRED COMPONENTS core imgcodecs)

add_subdirectory(bin)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(third_party/googletest)

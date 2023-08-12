cmake_minimum_required(VERSION 3.7...3.27)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(goop
        VERSION 1.0
        DESCRIPTION "Experimental Golang compiler and tooling"
        LANGUAGES CXX
        )

if (MSVC)
    add_compile_options(/W4)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

add_executable(goop driver/main.cpp)

find_package(Boost REQUIRED)
find_package(ICU COMPONENTS data io uc tu REQUIRED)

add_library(goop-parse parse/parser.cpp parse/tokens.cpp)
target_include_directories(goop-parse PUBLIC parse)
target_include_directories(goop-parse PUBLIC ${ICU_INCLUDE_DIRS})
target_link_libraries(goop-parse ${ICU_LIBRARIES})
target_include_directories(goop-parse PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(goop-parse ${Boost_LIBRARIES})

target_include_directories(goop PUBLIC driver)
target_link_libraries(goop goop-parse)

add_executable(goop-tok tools/tok/main.cpp)
target_link_libraries(goop-tok PUBLIC goop-parse)

add_subdirectory(test)

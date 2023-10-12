set(GOOP_PARSE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/tokens.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ast_debug.cpp
    )

add_subdirectory(parser)

add_library(goop-parse ${GOOP_PARSE_SOURCES})

target_include_directories(goop-parse PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(goop-parse PUBLIC ${ICU_INCLUDE_DIRS})
target_link_libraries(goop-parse ${ICU_LIBRARIES})
target_include_directories(goop-parse PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(goop-parse ${Boost_LIBRARIES})

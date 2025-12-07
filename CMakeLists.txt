cmake_minimum_required(VERSION 3.15)

project(sekai-deck-recommend-cpp VERSION 0.2.6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/json/single_include)

file(GLOB_RECURSE SRC_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_subdirectory(3rdparty/pybind11)

pybind11_add_module(sekai_deck_recommend ${SRC_FILES})

if(MSVC)
    target_compile_options(sekai_deck_recommend PRIVATE /W4)
else()
    target_compile_options(sekai_deck_recommend PRIVATE -Wall -Wextra -Wuninitialized)
endif()

install(TARGETS sekai_deck_recommend DESTINATION .)
install(FILES ${PROJECT_SOURCE_DIR}/src/sekai_deck_recommend.pyi DESTINATION .)

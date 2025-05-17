cmake_minimum_required(VERSION 3.15)

project(sekai-deck-recommend-cpp VERSION 0.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wuninitialized -O3")

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/json/single_include)
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/pybind11/include)

file(GLOB_RECURSE SRC_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_subdirectory(3rdparty/pybind11)
pybind11_add_module(sekai_deck_recommend ${SRC_FILES})

install(TARGETS sekai_deck_recommend DESTINATION .)
install(FILES ${PROJECT_SOURCE_DIR}/src/sekai_deck_recommend.pyi DESTINATION .)

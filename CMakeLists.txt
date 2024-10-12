cmake_minimum_required(VERSION 3.5.0)
project(black-scholes VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

add_executable(black-scholes main.cpp optionData.cpp stockData.cpp)

target_link_libraries(black-scholes ${CURL_LIBRARIES})
cmake_minimum_required(VERSION 3.17)
project(CoLisp VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_FLAGS " -Wall -Werror -Wextra -Wpedantic -fpermissive -ftemplate-depth=65536")
set(CMAKE_CXX_STANDARD 17)
add_executable(colisp_test src/main.cpp)

include_directories(include)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

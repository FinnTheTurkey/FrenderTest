cmake_minimum_required(VERSION 3.0.0)
project(FrenderPlayground VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(FrenderPlayground main.cpp)

add_subdirectory(Frender)
target_link_libraries(FrenderPlayground STATIC Frender)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

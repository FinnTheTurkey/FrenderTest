cmake_minimum_required(VERSION 3.0.0)
project(FrenderPlayground VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)

add_executable(FrenderPlayground main.cpp GLTFLoader.cc)

add_subdirectory(Frender)
target_link_libraries(FrenderPlayground Frender)
target_include_directories(FrenderPlayground PUBLIC .)

add_compile_definitions(GLM_FORCE_CTOR_INIT)

set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(assimp)
target_link_libraries(FrenderPlayground assimp)

install(TARGETS assimp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

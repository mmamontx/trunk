cmake_minimum_required(VERSION 3.14)
project(balloc2)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()

add_executable(
  test_balloc2
  test_balloc2.cpp
)
target_link_libraries(
  test_balloc2
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_balloc2)


find_package(Doxygen)

set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/balloc2.doxygen)
set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)
configure_file(${doxyfile_in} ${doxyfile} @ONLY)

add_custom_target(
  doc ALL
  COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile_in}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Generating API documentation with Doxygen"
  VERBATIM
)


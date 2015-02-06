cmake_minimum_required(VERSION 2.8.10)
project(gtest)

set(GTEST_SOURCES gtest/src/gtest-all.cc
gtest/src/gtest_main.cc)

set(GTEST_HEADERS gtest/gtest/gtest-death-test.h
gtest/gtest/gtest.h
gtest/gtest/gtest-message.h
gtest/gtest/gtest-param-test.h
gtest/gtest/gtest_pred_impl.h
gtest/gtest/gtest-printers.h
gtest/gtest/gtest_prod.h
gtest/gtest/gtest-spi.h
gtest/gtest/gtest-test-part.h
gtest/gtest/gtest-typed-test.h)

add_library(gtest ${GTEST_SOURCES} ${GTEST_HEADERS})
target_include_directories(gtest PRIVATE 
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/gtest/>")

add_subdirectory("${PROJECT_SOURCE_DIR}/functionaly/")


project(lite3d_tests)

unset(SOURCES_LIST)
unset(HEADERS_LIST)
source_files(".")
headers_files(".")

add_executable(lite3d_tests ${SOURCES_LIST} ${HEADERS_LIST})

target_include_directories(lite3d_tests PRIVATE 
"$<BUILD_INTERFACE:${CMAKE_LITE3D_TOP_DIR}/tests/gtest>"
"$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>")

if(MSVC)
target_compile_definitions(lite3d_tests PRIVATE
"$<BUILD_INTERFACE:_WINEXE>")
else()
target_compile_options(lite3d_tests PRIVATE 
"$<BUILD_INTERFACE:${SDL2_CFLAGS}>")
endif()

target_link_libraries(lite3d_tests 
gtest
lite3d
lite3dpp)

add_test(NAME lite3d_tests 
	COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lite3d_tests
	WORKING_DIRECTORY ${CMAKE_LITE3D_TOP_DIR}/media/)

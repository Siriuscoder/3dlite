
#	This file is part of lite3d (Light-weight 3d engine).
#	Copyright (C) 2024  Sirius (Korolev Nikita)
#
#	Lite3D is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	Lite3D is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with Lite3D.  If not, see <http://www.gnu.org/licenses/>.

project(gtest)

enable_testing()

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


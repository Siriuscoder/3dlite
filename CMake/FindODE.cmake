#  ODE_FOUND - system has ODE
#  ODE_INCLUDE_DIR - the ODE include directory
#  ODE_LIBRARY - Link these to use ODE

if(MSVC)
set(ODE_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/deps/ode-0.13/")
endif()

FIND_PATH(ODE_INCLUDE_DIR ode/ode.h
	$ENV{ODE_HOME}/include
	$ENV{ODE_HOME}
	$ENV{ODE_HOME}/..
	/usr/local/include/ode
	/usr/local/include
	/usr/include/ode
	/usr/include
	/opt/local/include/ode # DarwinPorts
	/opt/local/include
	/opt/csw/include/ode # Blastwave
	/opt/csw/include
	/opt/include/ode
	/opt/include
    ${ODE_SEARCH_PATHS}/include
	)

FIND_LIBRARY(ODE_LIBRARY_RELEASE 
	NAMES ode ode_single
	PATHS
	$ENV{ODE_HOME}/lib
	/usr/local/lib
	/usr/lib
	/sw/lib
	/opt/local/lib
	/opt/csw/lib
	/opt/lib
    ${ODE_SEARCH_PATHS}/lib/ReleaseSingleDLL
	)

SET(ODE_FOUND "NO")
IF(ODE_INCLUDE_DIR AND ODE_LIBRARY_RELEASE)
	SET(ODE_FOUND "YES")
	SET(ODE_LIBRARY ${ODE_LIBRARY_RELEASE})
ENDIF(ODE_INCLUDE_DIR AND ODE_LIBRARY_RELEASE)


message(STATUS "ODE inc: ${ODE_INCLUDE_DIR}")
message(STATUS "ODE lib: ${ODE_LIBRARY}")

mark_as_advanced(ODE_LIBRARY_RELEASE ODE_INCLUDE_DIR)

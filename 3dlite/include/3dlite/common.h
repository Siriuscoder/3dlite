/* 
 * File:   common.h
 * Author: sirius
 *
 * Created on 9 Ноябрь 2014 г., 14:04
 */

#ifndef COMMON_H
#define	COMMON_H

#include <stdint.h>
#include <stddef.h>

#ifdef	__cplusplus
#   define LITE3D_EXTERN extern "C" 
#else
#   define LITE3D_EXTERN
#endif

#if PLATFORM == WINDOWS

// Stuff for Visual C++ only
#	if defined(_MSC_VER)
// Disable whining about using 'this' as a member initializer on VC++.
#		pragma warning(disable: 4355)
#	endif


#	ifdef _WINDLL
		// When making the DLL, export tagged symbols, so they appear
		// in the import library.
#		define SOLA_EXPORT __declspec(dllexport)
#	elif !defined(_WINDLL)
		// We must be _using_ the DLL, so import symbols instead.
#		define SOLA_EXPORT __declspec(dllimport)
#	endif

#else
	// If not Windows, we assume some sort of Unixy build environment,
	// where autotools is used.  (This includes Cygwin!)  #include the
	// config.h file only if this file was included from a non-header
	// file, because headers must not be dependent on config.h.


	// Make DLL stuff a no-op on this platform.
#define LITE3D_EXPORT
#endif


#endif	/* COMMON_H */


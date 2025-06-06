 # Copyright (C) 2021-2025, Advanced Micro Devices. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions are met:
 # 1. Redistributions of source code must retain the above copyright notice,
 #    this list of conditions and the following disclaimer.
 # 2. Redistributions in binary form must reproduce the above copyright notice,
 #    this list of conditions and the following disclaimer in the documentation
 #    and/or other materials provided with the distribution.
 # 3. Neither the name of the copyright holder nor the names of its contributors
 #    may be used to endorse or promote products derived from this software
 # without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 # AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 # ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 # LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 # CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 # SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 # INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 # CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 # ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 # POSSIBILITY OF SUCH DAMAGE.

include(TestBigEndian)

CMAKE_MINIMUM_REQUIRED(VERSION 3.26)

set(PROJECT_FULL_NAME "AOCL-Cryptography")
set(PROJECT_NAME "ALCP")

SET(CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
  # Add default path
  ${CMAKE_MODULE_PATH}
  )

INCLUDE(AlcpUtils)
INCLUDE(AlcpConfig)

# OS Specific includes
IF(UNIX)
  INCLUDE(CompilerLinux)
ELSEIF(WIN32)
  INCLUDE(CompilerWin)
ENDIF(UNIX)

# Set the release version
set(AOCL_RELEASE_VERSION "5.1.0" CACHE STRING "AOCL RELEASE VERSION")

PROJECT (ALCP
         VERSION ${AOCL_RELEASE_VERSION}
		 DESCRIPTION "AOCL Cryptographic Primitives"
		 LANGUAGES C CXX
	    )

include(GNUInstallDirs)


# check for compiler version
alcp_check_compiler_version()

# check build environment
alcp_get_build_environment()

# check for 7zip installation, needed by static library creation on linux
if(UNIX)
	check_7zip_installed()
endif(UNIX)

# SPECIFY THE C STANDARD
SET(CMAKE_C_STANDARD 11)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_C_STANDARD_REQUIRED TRUE)
SET(CMAKE_CXX_STANDARD_REQUIRED TRUE)


IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	message(STATUS "CMAKE_BUILD_TYPE set to Release by default.")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

IF(ALCP_ENABLE_CLANG_TIDY)
	IF(NOT CMAKE_CXX_CLANG_TIDY)
		FIND_PROGRAM(CLANG_TIDY "clang-tidy-12")
		SET(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY}")
	ENDIF(NOT CMAKE_CXX_CLANG_TIDY)
ENDIF(ALCP_ENABLE_CLANG_TIDY)

SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )

# Make the static and shared libraries are built in top most directory
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

# Install Options
OPTION(ALCP_INSTALL_COMBINED_STATIC "INSTALL ALCP+ALCI COMBINED STATIC LIB" OFF)

IF(NOT DEFINED ENABLE_AOCL_UTILS)
 	MESSAGE(STATUS "Enabling utils as it's needs to be enabled for proper dispatching")
ENDIF()


# Options which should not be modified
MARK_AS_ADVANCED(CMAKE_INSTALL_LIBDIR)

# santizer options
OPTION(ALCP_SANITIZE "COMPILE WITH SANITIZER SUPPORT TO DETECT MEMORY ERRORS" OFF)


# All the options which are available with ALCP
# These options can be listed with cmake -LH
OPTION(ALCP_ENABLE_HTML "ENABLE HTML DOCUMENT GENERATION" OFF)
IF(ALCP_ENABLE_HTML)
	OPTION(ALCP_ENABLE_DOXYGEN "ENABLE DOXYGEN DOCUMENTATION GENERATION" ON)
	OPTION(ALCP_ENABLE_SPHINX "ENABLE SPHINX DOCUMENTATION GENERATION" ON)
ENDIF()
OPTION(ALCP_ENABLE_CLANG_TIDY "ENABLE CLANG TIDY" OFF)
OPTION(ALCP_ENABLE_DYNAMIC_COMPILER_PICK "ENABLE DYNAMIC COMPILER" ON)
IF(ALCP_SANITIZE)
	SET(ALCP_ENABLE_DYNAMIC_COMPILER_PICK OFF)
	MESSAGE(WARNING "DYNAMIC COMPILER is disabled, since Santizer is ON")
ENDIF(ALCP_SANITIZE)
OPTION(ALCP_ENABLE_TESTS "ENABLE TESTING" OFF)
# Enabling fuzz tests
OPTION(ALCP_ENABLE_FUZZ_TESTS "ENABLE FUZZ TESTING" OFF)
OPTION(ALCP_ENABLE_BENCH "ENABLE BENCHMARKING" OFF)
OPTION(ALCP_ENABLE_EXAMPLES "ENABLE EXAMPLES" ON)
OPTION(ENABLE_AOCL_UTILS "ENABLE AOCL UTILS SUPPORT FOR CPUID BASED DISPATCHING" ON)

set(AOCL_UTILS_INSTALL_DIR "" CACHE STRING "AOCL UTILS INSTALLED DIRECTORY")
set(AOCL_COMPAT_LIBS "" CACHE STRING "COMPATIBILITY LAYER FOR OPENSSL AND IPP")

# for parsing aocl compat libs options
SET(ENABLE_OPENSSL_COMPAT FALSE)
SET(ENABLE_IPP_COMPAT FALSE)
IF (AOCL_COMPAT_LIBS)
	INCLUDE(${CMAKE_SOURCE_DIR}/cmake/AlcpCompat.cmake)
	alcp_check_compat_option()
endif (AOCL_COMPAT_LIBS)

IF (NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/tests)
	SET(ALCP_ENABLE_TESTS OFF)
	MESSAGE(STATUS "Disabling tests")
	SET (ALCP_ENABLE_BENCH OFF)
	MESSAGE(STATUS "Disabling bench")
endif()

# set proj version
set(PROJECT_VERSION ${AOCL_RELEASE_VERSION})

# Testing/Benchmarking options
OPTION(ENABLE_TESTS_IPP_API "ENABLE IPP CALLS IN TESTING AND BENCHMARKING" OFF)
OPTION(ENABLE_TESTS_OPENSSL_API "ENABLE OPENSSL CALLS IN TESTING AND BENCHMARKING" OFF)
set(OPENSSL_INSTALL_DIR "" CACHE STRING "OPENSSL INSTALLED DIRECTORY FOR TESTING AND BENCHMARKING")
set(IPP_INSTALL_DIR "" CACHE STRING "IPPCP INSTALLED DIRECTORY FOR TESTING AND BENCHMARKING")

if(ALCP_SANITIZE)
	alcp_add_sanitize_flags()
endif()

OPTION(ALCP_MEMCHECK_VALGRIND "RUN ALCP TESTS WITH VALGRIND MEMORY CHECK TO DETECT MEMORY ERRORS" OFF)

IF (NOT DEFINED ALCP_BIGNUM_USE_OPENSSL)
	SET (ALCP_BIGNUM_USE_OPENSSL ON)
ENDIF()

IF (NOT DEFINED ALCP_DISABLE_ASSEMBLY)
	SET (ALCP_DISABLE_ASSEMBLY OFF)
ENDIF()

IF (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	SET (ALCP_DISABLE_ASSEMBLY ON)
ENDIF()

# Generate Configuration config.h
GEN_CONF()

IF (NOT DEFINED BUILD_SHARED_LIBS)
	SET (BUILD_SHARED_LIBS ON)
ENDIF()

SET(CMAKE_DEBUG_POSTFIX _DEBUG)

IF(ALCP_ENABLE_CCM_MULTI_UPDATE)
	ADD_COMPILE_OPTIONS("-DCCM_MULTI_UPDATE")
ENDIF(ALCP_ENABLE_CCM_MULTI_UPDATE)

# gcm always compute table optimization flag
OPTION(ALCP_ENABLE_GCM_ALWAYS_COMPUTE_TABLE "ENABLE THIS FOR PERFORMANCE UPLIFT IN APPLICATIONS USING AES-GCM" ON)

IF(ALCP_ENABLE_GCM_ALWAYS_COMPUTE_TABLE)
	message(STATUS "ALCP_ENABLE_GCM_ALWAYS_COMPUTE_TABLE is enabled")
	ADD_COMPILE_OPTIONS("-DGCM_ALWAYS_COMPUTE")
ENDIF(ALCP_ENABLE_GCM_ALWAYS_COMPUTE_TABLE)

ADD_SUBDIRECTORY(lib)

IF(ALCP_ENABLE_HTML)
IF(ALCP_ENABLE_DOXYGEN)
	include(${CMAKE_SOURCE_DIR}/cmake/doxygen.cmake)
ENDIF(ALCP_ENABLE_DOXYGEN)
IF(ALCP_ENABLE_SPHINX)
	include(${CMAKE_SOURCE_DIR}/sphinx/CMakeLists.txt)
ENDIF(ALCP_ENABLE_SPHINX)
ENDIF(ALCP_ENABLE_HTML)

# building examples
IF (ALCP_ENABLE_EXAMPLES)
	IF (NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/examples)
		MESSAGE(FATAL_ERROR "Examples dir not found!")
        ENDIF()
	ADD_SUBDIRECTORY(examples)
	MESSAGE(STATUS "Building examples")
ENDIF()

# building tests
IF (ALCP_ENABLE_TESTS)
	ENABLE_TESTING()
	IF (NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/tests)
    	MESSAGE(FATAL_ERROR "Tests folder not found!")
	ENDIF()
	ADD_SUBDIRECTORY(tests)
  MESSAGE(STATUS "Enabling Tests")
ENDIF()

# building test bench
IF (ALCP_ENABLE_BENCH)
	# bench wont compile without tests enabled!
	IF (NOT ALCP_ENABLE_TESTS)
		MESSAGE(FATAL_ERROR "Tests need to be enabled for Benchmarks!(set ALCP_ENABLE_TESTS to ON)")
	endif()
	# check if bench dir exists
	IF (NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/bench)
    	MESSAGE(FATAL_ERROR "Bench folder not found!")
    ENDIF()
	ADD_SUBDIRECTORY(bench)
	MESSAGE(STATUS "Enabling Benchmarks")
ENDIF()

INSTALL(TARGETS alcp)

# install compat libs if option is passed
IF (ENABLE_OPENSSL_COMPAT)
	INSTALL(TARGETS openssl-compat)
ENDIF()
IF (ENABLE_IPP_COMPAT)
	INSTALL(TARGETS ipp-compat)
ENDIF()

INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/include/alcp
        TYPE INCLUDE
		)

IF(UNIX AND ALCP_INSTALL_COMBINED_STATIC)
		# FIXME: SUFIX Debug will be ignored
		INSTALL(FILES ${CMAKE_BINARY_DIR}/libalcp_static.a DESTINATION lib RENAME libalcp.a)
ELSE()
	INSTALL(TARGETS alcp_static)
ENDIF(UNIX AND ALCP_INSTALL_COMBINED_STATIC)

IF (ALCP_ENABLE_EXAMPLES)
	# examples which should be part of release package
	FILE(GLOB EXAMPLES_FILES_CIPHER
			${CMAKE_CURRENT_SOURCE_DIR}/examples/cipher/*.c)
	FILE(GLOB EXAMPLES_FILES_DIGEST
			${CMAKE_CURRENT_SOURCE_DIR}/examples/digest/*.c)
	FILE(GLOB EXAMPLES_FILES_MAC
			${CMAKE_CURRENT_SOURCE_DIR}/examples/mac/*.c)
	FILE(GLOB EXAMPLES_FILES_ECDH
			${CMAKE_CURRENT_SOURCE_DIR}/examples/ecdh/*.c)
	FILE(GLOB EXAMPLES_FILES_RNG
			${CMAKE_CURRENT_SOURCE_DIR}/examples/rng/*.c)
	FILE(GLOB EXAMPLES_FILES_RSA
			${CMAKE_CURRENT_SOURCE_DIR}/examples/rsa/*.c)
	FILE(GLOB EXAMPLES_FILES_VERSION
			${CMAKE_CURRENT_SOURCE_DIR}/examples/version/*.c)
	FILE(GLOB EXAMPLES_FILES_MISC
			${CMAKE_CURRENT_SOURCE_DIR}/examples/misc/*.c)
    FILE(GLOB EXAMPLES_README
			${CMAKE_CURRENT_SOURCE_DIR}/examples/BUILD_Examples.md)

	# install
	INSTALL(FILES ${EXAMPLES_FILES_CIPHER}
			DESTINATION examples/cipher)
	INSTALL(FILES ${EXAMPLES_FILES_DIGEST}
			DESTINATION examples/digest)
	INSTALL(FILES ${EXAMPLES_FILES_MAC}
			DESTINATION examples/mac)
	INSTALL(FILES ${EXAMPLES_FILES_ECDH}
			DESTINATION examples/ecdh)
	INSTALL(FILES ${EXAMPLES_FILES_VERSION}
			DESTINATION examples/version)
	INSTALL(FILES ${EXAMPLES_FILES_RNG}
			DESTINATION examples/rng)
	INSTALL(FILES ${EXAMPLES_FILES_RSA}
			DESTINATION examples/rsa)
	#TODO: do we need to cover this under install?
	# INSTALL(FILES ${EXAMPLES_FILES_MISC}
	# 		DESTINATION examples/misc)
	INSTALL(FILES examples/Makefile
			DESTINATION .)
	INSTALL(FILES ${EXAMPLES_README}
			DESTINATION .)
ENDIF (ALCP_ENABLE_EXAMPLES)

# docs, etc which should be part of the package
FILE(GLOB INSTALL_DOCS_REL_NOTES
			${CMAKE_CURRENT_SOURCE_DIR}/docs/ReleaseNotes.txt)
FILE(GLOB INSTALL_DOCS_COMPAT
			${CMAKE_CURRENT_SOURCE_DIR}/docs/compat/*.md)
FILE(GLOB INSTALL_DOCS_LICENSE
			${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
FILE(GLOB INSTALL_DOCS_README
			${CMAKE_CURRENT_SOURCE_DIR}/README.md)

# install docs
INSTALL(FILES ${INSTALL_DOCS_REL_NOTES}
		DESTINATION .)
INSTALL(FILES ${INSTALL_DOCS_COMPAT}
		DESTINATION docs)
INSTALL(FILES ${INSTALL_DOCS_README}
		DESTINATION docs)
INSTALL(FILES ${INSTALL_DOCS_LICENSE}
		DESTINATION .)

# print configure summary
MESSAGE(STATUS "\n--------------------Configure Summary--------------------\n")

# function to filter and print variables based on wildcard
FUNCTION (GetVars _prefix)
    GET_CMAKE_PROPERTY(_vars VARIABLES)
    STRING (REGEX MATCHALL "(^|)${_prefix}[A-Za-z0-9_]*" matches "${_vars}")
	FOREACH (var ${matches})
        IF (${var})
			MESSAGE(STATUS "${var}................${${var}}")
	 	ENDIF()
	ENDFOREACH()
ENDFUNCTION()

# print cmake vars
MESSAGE (STATUS "PROJECT ............................... " ${PROJECT_FULL_NAME})
MESSAGE (STATUS "PROJECT VERSION ....................... " ${PROJECT_VERSION})
MESSAGE (STATUS "CMAKE_BINARY_DIR ...................... " ${CMAKE_BINARY_DIR})
MESSAGE (STATUS "ALCP_CRYPTO_DIR ....................... " ${CMAKE_CURRENT_SOURCE_DIR})
MESSAGE (STATUS "CMAKE_GENERATOR ....................... " ${CMAKE_GENERATOR})
MESSAGE (STATUS "CMAKE_C_COMPILER_ID ................... " ${CMAKE_C_COMPILER_ID})
MESSAGE (STATUS "CMAKE_CXX_COMPILER_ID ................. " ${CMAKE_CXX_COMPILER_ID})
MESSAGE (STATUS "CMAKE_INSTALL_PREFIX .................. " ${CMAKE_INSTALL_PREFIX})
MESSAGE (STATUS "CMAKE version ......................... " ${CMAKE_VERSION})
MESSAGE (STATUS "CMAKE Compiler ID ..................... " ${CMAKE_CXX_COMPILER_ID})
MESSAGE (STATUS "CMAKE C COMPILER ...................... " ${CMAKE_C_COMPILER})
MESSAGE (STATUS "CMAKE CXX COMPILER .................... " ${CMAKE_CXX_COMPILER})
MESSAGE (STATUS "CMAKE C Compiler version .............. " ${CMAKE_C_COMPILER_VERSION})
MESSAGE (STATUS "CMAKE CXX Compiler version ............ " ${CMAKE_CXX_COMPILER_VERSION})
MESSAGE (STATUS "CMAKE CXX Flags Release ............... " ${CMAKE_CXX_FLAGS_RELEASE})
MESSAGE (STATUS "CMAKE CXX Flags Debug: ................ " ${CMAKE_CXX_FLAGS_DEBUG})
MESSAGE (STATUS "CMAKE BUILD TYPE ...................... " ${CMAKE_BUILD_TYPE})
MESSAGE (STATUS "CMAKE CXX STANDARD .................... " ${CMAKE_CXX_STANDARD})

# alcp vars
# FIXME: better to have all the below arguments starting with alcp_*,
GetVars("ALCP_")
GetVars("AOCL_")
GetVars("IPP_")
GetVars("OPENSSL_")
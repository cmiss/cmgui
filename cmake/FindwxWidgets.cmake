# - Find a wxWidgets (a.k.a., wxWindows) installation.
# This module finds if wxWidgets is installed and selects a default
# configuration to use. wxWidgets is a modular library. To specify the
# modules that you will use, you need to name them as components to
# the package:
# 
# FIND_PACKAGE(wxWidgets COMPONENTS base core ...)
# 
# There are two search branches: a windows style and a unix style. For
# windows, the following variables are searched for and set to
# defaults in case of multiple choices. Change them if the defaults
# are not desired (i.e., these are the only variables you should
# change to select a configuration):
#
#  wxWidgets_ROOT_DIR      - Base wxWidgets directory
#                            (e.g., C:/wxWidgets-2.6.3).
#  wxWidgets_LIB_DIR       - Path to wxWidgets libraries
#                            (e.g., C:/wxWidgets-2.6.3/lib/vc_lib).
#  wxWidgets_CONFIGURATION - Configuration to use
#                            (e.g., msw, mswd, mswu, mswunivud, etc.)
# 
# For unix style it uses the wx-config utility. You can select between
# debug/release, unicode/ansi, universal/non-universal, and
# static/shared in the QtDialog or ccmake interfaces by turning ON/OFF
# the following variables:
#
#  wxWidgets_USE_DEBUG
#  wxWidgets_USE_UNICODE
#  wxWidgets_USE_UNIVERSAL
#  wxWidgets_USE_STATIC
#  
# The following are set after the configuration is done for both
# windows and unix style:
#
#  wxWidgets_FOUND            - Set to TRUE if wxWidgets was found.
#  wxWidgets_INCLUDE_DIRS     - Include directories for WIN32
#                               i.e., where to find "wx/wx.h" and
#                               "wx/setup.h"; possibly empty for unices.
#  wxWidgets_LIBRARIES        - Path to the wxWidgets libraries.
#  wxWidgets_LIBRARY_DIRS     - compile time link dirs, useful for
#                               rpath on UNIX. Typically an empty string
#                               in WIN32 environment.
#  wxWidgets_DEFINITIONS      - Contains defines required to compile/link
#                               against WX, e.g. WXUSINGDLL
#  wxWidgets_DEFINITIONS_DEBUG- Contains defines required to compile/link
#                               against WX debug builds, e.g. __WXDEBUG__
#  wxWidgets_CXX_FLAGS        - Include dirs and compiler flags for
#                               unices, empty on WIN32. Essentially
#                               "`wx-config --cxxflags`".
#  wxWidgets_USE_FILE         - Convenience include file.
#
# Sample usage:
#   FIND_PACKAGE(wxWidgets COMPONENTS base core gl net)
#   IF(wxWidgets_FOUND)
#     INCLUDE(${wxWidgets_USE_FILE})
#     # and for each of your dependant executable/library targets:
#     TARGET_LINK_LIBRARIES(<YourTarget> ${wxWidgets_LIBRARIES})
#   ENDIF(wxWidgets_FOUND)
#
# If wxWidgets is required (i.e., not an optional part):
#   FIND_PACKAGE(wxWidgets REQUIRED base core gl net)
#   INCLUDE(${wxWidgets_USE_FILE})
#   # and for each of your dependant executable/library targets:
#   TARGET_LINK_LIBRARIES(<YourTarget> ${wxWidgets_LIBRARIES})

#=============================================================================
# Copyright 2004-2009 Kitware, Inc.
# Copyright 2007-2009 Miguel A. Figueroa-Villanueva <miguelf at ieee dot org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

#
# FIXME: check this and provide a correct sample usage...
#        Remember to connect back to the upper text.
# Sample usage with monolithic wx build:
#
#   FIND_PACKAGE(wxWidgets COMPONENTS mono)
#   ...

# NOTES
#
# This module has been tested on the WIN32 platform with wxWidgets
# 2.6.2, 2.6.3, and 2.5.3. However, it has been designed to
# easily extend support to all possible builds, e.g., static/shared,
# debug/release, unicode, universal, multilib/monolithic, etc..
#
# If you want to use the module and your build type is not supported
# out-of-the-box, please contact me to exchange information on how
# your system is setup and I'll try to add support for it.
#
# AUTHOR
#
# Miguel A. Figueroa-Villanueva (miguelf at ieee dot org).
# Jan Woetzel (jw at mip.informatik.uni-kiel.de).
#
# Based on previous works of:
# Jan Woetzel (FindwxWindows.cmake),
# Jorgen Bodde and Jerry Fath (FindwxWin.cmake).

# TODO/ideas
#
# (1) Option/Setting to use all available wx libs
# In contrast to expert developer who lists the
# minimal set of required libs in wxWidgets_USE_LIBS
# there is the newbie user:
#   - who just wants to link against WX with more 'magic'
#   - doesn't know the internal structure of WX or how it was built,
#     in particular if it is monolithic or not
#   - want to link against all available WX libs
# Basically, the intent here is to mimic what wx-config would do by
# default (i.e., `wx-config --libs`).
#
# Possible solution:
#   Add a reserved keyword "std" that initializes to what wx-config
# would default to. If the user has not set the wxWidgets_USE_LIBS,
# default to "std" instead of "base core" as it is now. To implement
# "std" will basically boil down to a FOR_EACH lib-FOUND, but maybe
# checking whether a minimal set was found.


# FIXME: This and all the DBG_MSG calls should be removed after the
# module stabilizes.
# 
# Helper macro to control the debugging output globally. There are
# two versions for controlling how verbose your output should be.
MACRO(DBG_MSG _MSG)
#  MESSAGE(STATUS
#    "${CMAKE_CURRENT_LIST_FILE}(${CMAKE_CURRENT_LIST_LINE}): ${_MSG}")
ENDMACRO(DBG_MSG)
MACRO(DBG_MSG_V _MSG)
#  MESSAGE(STATUS
#    "${CMAKE_CURRENT_LIST_FILE}(${CMAKE_CURRENT_LIST_LINE}): ${_MSG}")
ENDMACRO(DBG_MSG_V)

# Clear return values in case the module is loaded more than once.
# SET(wxWidgets_FOUND FALSE)
# SET(wxWidgets_INCLUDE_DIRS "")
# SET(wxWidgets_LIBRARIES    "")
# SET(wxWidgets_LIBRARY_DIRS "")
# SET(wxWidgets_CXX_FLAGS    "")

# Using SYSTEM with INCLUDE_DIRECTORIES in conjunction with wxWidgets on
# the Mac produces compiler errors. Set wxWidgets_INCLUDE_DIRS_NO_SYSTEM
# to prevent UsewxWidgets.cmake from using SYSTEM.
#
# See cmake mailing list discussions for more info:
#   http://www.cmake.org/pipermail/cmake/2008-April/021115.html
#   http://www.cmake.org/pipermail/cmake/2008-April/021146.html
#
IF(APPLE)
	SET(wxWidgets_INCLUDE_DIRS_NO_SYSTEM 1)
ENDIF(APPLE)

# DEPRECATED: This is a patch to support the DEPRECATED use of
# wxWidgets_USE_LIBS.
#
# If wxWidgets_USE_LIBS is set:
# - if using <components>, then override wxWidgets_USE_LIBS
# - else set wxWidgets_FIND_COMPONENTS to wxWidgets_USE_LIBS
IF(wxWidgets_USE_LIBS AND NOT wxWidgets_FIND_COMPONENTS)
	SET(wxWidgets_FIND_COMPONENTS ${wxWidgets_USE_LIBS})
ENDIF(wxWidgets_USE_LIBS AND NOT wxWidgets_FIND_COMPONENTS)
DBG_MSG("wxWidgets_FIND_COMPONENTS : ${wxWidgets_FIND_COMPONENTS}")

# Add the convenience use file if available.
#
# Get dir of this file which may reside in:
# - CMAKE_MAKE_ROOT/Modules on CMake installation
# - CMAKE_MODULE_PATH if user prefers his own specialized version
SET(wxWidgets_USE_FILE "")
GET_FILENAME_COMPONENT(
	wxWidgets_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
# Prefer an existing customized version, but the user might override
# the FindwxWidgets module and not the UsewxWidgets one.
IF(EXISTS "${wxWidgets_CURRENT_LIST_DIR}/UsewxWidgets.cmake")
	SET(wxWidgets_USE_FILE
	"${wxWidgets_CURRENT_LIST_DIR}/UsewxWidgets.cmake")
ELSE(EXISTS "${wxWidgets_CURRENT_LIST_DIR}/UsewxWidgets.cmake")
	SET(wxWidgets_USE_FILE UsewxWidgets.cmake)
ENDIF(EXISTS "${wxWidgets_CURRENT_LIST_DIR}/UsewxWidgets.cmake")

#=====================================================================
#=====================================================================
IF(WIN32 AND NOT CYGWIN AND NOT MSYS)
	SET(wxWidgets_FIND_STYLE "win32")
ELSE(WIN32 AND NOT CYGWIN AND NOT MSYS)
	IF(UNIX OR MSYS)
		SET(wxWidgets_FIND_STYLE "unix")
	ENDIF(UNIX OR MSYS)
ENDIF(WIN32 AND NOT CYGWIN AND NOT MSYS)

#=====================================================================
# WIN32_FIND_STYLE
#=====================================================================
IF(wxWidgets_FIND_STYLE STREQUAL "win32")
	SET( wxWidgets_DIR "" CACHE PATH "Where is wxWidgets installed?" )
	INCLUDE( ${wxWidgets_DIR}/wxWidgets-config.cmake )
#=====================================================================
# UNIX_FIND_STYLE
#=====================================================================
ELSEIF(wxWidgets_FIND_STYLE STREQUAL "unix")
	#-----------------------------------------------------------------
	# UNIX: Helper MACROS
	#-----------------------------------------------------------------
	#
	# Set the default values based on "wx-config --selected-config".
	#
	MACRO(WX_CONFIG_SELECT_GET_DEFAULT)
		EXECUTE_PROCESS(
			COMMAND sh "${wxWidgets_CONFIG_EXECUTABLE}" --selected-config
			OUTPUT_VARIABLE _wx_selected_config
			RESULT_VARIABLE _wx_result
			ERROR_QUIET
			)
		IF(_wx_result EQUAL 0)
			FOREACH(_opt_name debug static unicode universal)
			STRING(TOUPPER ${_opt_name} _upper_opt_name)
			IF(_wx_selected_config MATCHES ".*${_opt_name}.*")
				SET(wxWidgets_DEFAULT_${_upper_opt_name} ON)
			ELSE(_wx_selected_config MATCHES ".*${_opt_name}.*")
				SET(wxWidgets_DEFAULT_${_upper_opt_name} OFF)
			ENDIF(_wx_selected_config MATCHES ".*${_opt_name}.*")
			ENDFOREACH(_opt_name)
		ELSE(_wx_result EQUAL 0)
			FOREACH(_upper_opt_name DEBUG STATIC UNICODE UNIVERSAL)
			SET(wxWidgets_DEFAULT_${_upper_opt_name} OFF)
			ENDFOREACH(_upper_opt_name)
		ENDIF(_wx_result EQUAL 0)
	ENDMACRO(WX_CONFIG_SELECT_GET_DEFAULT)

	#
	# Query a boolean configuration option to determine if the system
	# has both builds available. If so, provide the selection option
	# to the user.
	#
	MACRO(WX_CONFIG_SELECT_QUERY_BOOL _OPT_NAME _OPT_HELP)
		EXECUTE_PROCESS(
			COMMAND sh "${wxWidgets_CONFIG_EXECUTABLE}" --${_OPT_NAME}=yes
			RESULT_VARIABLE _wx_result_yes
			OUTPUT_QUIET
			ERROR_QUIET
			)
		EXECUTE_PROCESS(
			COMMAND sh "${wxWidgets_CONFIG_EXECUTABLE}" --${_OPT_NAME}=no
			RESULT_VARIABLE _wx_result_no
			OUTPUT_QUIET
			ERROR_QUIET
			)
		STRING(TOUPPER ${_OPT_NAME} _UPPER_OPT_NAME)
		IF(_wx_result_yes EQUAL 0 AND _wx_result_no EQUAL 0)
			OPTION(wxWidgets_USE_${_UPPER_OPT_NAME}
			${_OPT_HELP} ${wxWidgets_DEFAULT_${_UPPER_OPT_NAME}})
		ELSE(_wx_result_yes EQUAL 0 AND _wx_result_no EQUAL 0)
			# If option exists (already in cache), force to available one.
			IF(DEFINED wxWidgets_USE_${_UPPER_OPT_NAME})
			IF(_wx_result_yes EQUAL 0)
				SET(wxWidgets_USE_${_UPPER_OPT_NAME} ON CACHE BOOL ${_OPT_HELP} FORCE)
			ELSE(_wx_result_yes EQUAL 0)
				SET(wxWidgets_USE_${_UPPER_OPT_NAME} OFF CACHE BOOL ${_OPT_HELP} FORCE)
			ENDIF(_wx_result_yes EQUAL 0)
			ENDIF(DEFINED wxWidgets_USE_${_UPPER_OPT_NAME})
		ENDIF(_wx_result_yes EQUAL 0 AND _wx_result_no EQUAL 0)
	ENDMACRO(WX_CONFIG_SELECT_QUERY_BOOL)

	# 
	# Set wxWidgets_SELECT_OPTIONS to wx-config options for selecting
	# among multiple builds.
	#
	MACRO(WX_CONFIG_SELECT_SET_OPTIONS)
		SET(wxWidgets_SELECT_OPTIONS "")
		FOREACH(_opt_name debug static unicode universal)
			STRING(TOUPPER ${_opt_name} _upper_opt_name)
			IF(DEFINED wxWidgets_USE_${_upper_opt_name})
			IF(wxWidgets_USE_${_upper_opt_name})
				LIST(APPEND wxWidgets_SELECT_OPTIONS --${_opt_name}=yes)
			ELSE(wxWidgets_USE_${_upper_opt_name})
				LIST(APPEND wxWidgets_SELECT_OPTIONS --${_opt_name}=no)
			ENDIF(wxWidgets_USE_${_upper_opt_name})
			ENDIF(DEFINED wxWidgets_USE_${_upper_opt_name})
		ENDFOREACH(_opt_name)
	ENDMACRO(WX_CONFIG_SELECT_SET_OPTIONS)

	#-----------------------------------------------------------------
	# UNIX: Start actual work.
	#-----------------------------------------------------------------
	# Support cross-compiling, only search in the target platform.
	FIND_PROGRAM(wxWidgets_CONFIG_EXECUTABLE wx-config
		ONLY_CMAKE_FIND_ROOT_PATH)

	IF(wxWidgets_CONFIG_EXECUTABLE)
		SET(wxWidgets_FOUND TRUE)

		# get defaults based on "wx-config --selected-config"
		WX_CONFIG_SELECT_GET_DEFAULT()

		# for each option: if both builds are available, provide option
		WX_CONFIG_SELECT_QUERY_BOOL(debug "Use debug build?")
		WX_CONFIG_SELECT_QUERY_BOOL(unicode "Use unicode build?")
		WX_CONFIG_SELECT_QUERY_BOOL(universal "Use universal build?")
		WX_CONFIG_SELECT_QUERY_BOOL(static "Link libraries statically?")

		# process selection to set wxWidgets_SELECT_OPTIONS
		WX_CONFIG_SELECT_SET_OPTIONS()
		DBG_MSG("wxWidgets_SELECT_OPTIONS=${wxWidgets_SELECT_OPTIONS}")

		# run the wx-config program to get cxxflags
		EXECUTE_PROCESS(
			COMMAND sh "${wxWidgets_CONFIG_EXECUTABLE}"
			${wxWidgets_SELECT_OPTIONS} --cxxflags
			OUTPUT_VARIABLE wxWidgets_CXX_FLAGS
			RESULT_VARIABLE RET
			ERROR_QUIET
			)
		IF(RET EQUAL 0)
			STRING(STRIP "${wxWidgets_CXX_FLAGS}" wxWidgets_CXX_FLAGS)
			SEPARATE_ARGUMENTS(wxWidgets_CXX_FLAGS)

			DBG_MSG_V("wxWidgets_CXX_FLAGS=${wxWidgets_CXX_FLAGS}")

			# parse definitions from cxxflags; drop -D* from CXXFLAGS and the -D prefix
			STRING(REGEX MATCHALL "-D[^;]+"
				wxWidgets_DEFINITIONS "${wxWidgets_CXX_FLAGS}")
			STRING(REGEX REPLACE "-D[^;]+;" ""
				wxWidgets_CXX_FLAGS "${wxWidgets_CXX_FLAGS}")
			STRING(REPLACE "-D" ""
				wxWidgets_DEFINITIONS "${wxWidgets_DEFINITIONS}")

			# parse include dirs from cxxflags; drop -I prefix
			STRING(REGEX MATCHALL "-I[^;]+"
			wxWidgets_INCLUDE_DIRS "${wxWidgets_CXX_FLAGS}")
			STRING(REGEX REPLACE "-I[^;]+;" ""
			wxWidgets_CXX_FLAGS "${wxWidgets_CXX_FLAGS}")
			STRING(REPLACE "-I" ""
			wxWidgets_INCLUDE_DIRS "${wxWidgets_INCLUDE_DIRS}")

			DBG_MSG_V("wxWidgets_DEFINITIONS=${wxWidgets_DEFINITIONS}")
			DBG_MSG_V("wxWidgets_INCLUDE_DIRS=${wxWidgets_INCLUDE_DIRS}")
			DBG_MSG_V("wxWidgets_CXX_FLAGS=${wxWidgets_CXX_FLAGS}")

		ELSE(RET EQUAL 0)
			SET(wxWidgets_FOUND FALSE)
			DBG_MSG_V(
			"${wxWidgets_CONFIG_EXECUTABLE} --cxxflags FAILED with RET=${RET}")
		ENDIF(RET EQUAL 0)

		# run the wx-config program to get the libs
		# - NOTE: wx-config doesn't verify that the libs requested exist
		# it just produces the names. Maybe a TRY_COMPILE would
		# be useful here...
		STRING(REPLACE ";" ","
			wxWidgets_FIND_COMPONENTS "${wxWidgets_FIND_COMPONENTS}")
		EXECUTE_PROCESS(
			COMMAND sh "${wxWidgets_CONFIG_EXECUTABLE}"
			${wxWidgets_SELECT_OPTIONS} --libs ${wxWidgets_FIND_COMPONENTS}
			OUTPUT_VARIABLE wxWidgets_LIBRARIES
			RESULT_VARIABLE RET
			ERROR_QUIET
			)
		IF(RET EQUAL 0)
			STRING(STRIP "${wxWidgets_LIBRARIES}" wxWidgets_LIBRARIES)
			SEPARATE_ARGUMENTS(wxWidgets_LIBRARIES)
			STRING(REPLACE "-framework;" "-framework "
			wxWidgets_LIBRARIES "${wxWidgets_LIBRARIES}")
			STRING(REPLACE "-arch;" "-arch "
			wxWidgets_LIBRARIES "${wxWidgets_LIBRARIES}")
			STRING(REPLACE "-isysroot;" "-isysroot "
			wxWidgets_LIBRARIES "${wxWidgets_LIBRARIES}")

			# extract linkdirs (-L) for rpath (i.e., LINK_DIRECTORIES)
			STRING(REGEX MATCHALL "-L[^;]+"
			wxWidgets_LIBRARY_DIRS "${wxWidgets_LIBRARIES}")
			STRING(REPLACE "-L" ""
			wxWidgets_LIBRARY_DIRS "${wxWidgets_LIBRARY_DIRS}")

			DBG_MSG_V("wxWidgets_LIBRARIES=${wxWidgets_LIBRARIES}")
			DBG_MSG_V("wxWidgets_LIBRARY_DIRS=${wxWidgets_LIBRARY_DIRS}")

		ELSE(RET EQUAL 0)
			SET(wxWidgets_FOUND FALSE)
			DBG_MSG("${wxWidgets_CONFIG_EXECUTABLE} --libs ${wxWidgets_FIND_COMPONENTS} FAILED with RET=${RET}")
		ENDIF(RET EQUAL 0)
	ENDIF(wxWidgets_CONFIG_EXECUTABLE)

#=====================================================================
# Neither UNIX_FIND_STYLE, nor WIN32_FIND_STYLE
#=====================================================================
ELSE(wxWidgets_FIND_STYLE STREQUAL "win32")
	IF(NOT wxWidgets_FIND_QUIETLY)
	  MESSAGE(STATUS
		"${CMAKE_CURRENT_LIST_FILE}(${CMAKE_CURRENT_LIST_LINE}): \n"
		"  Platform unknown/unsupported. It's neither WIN32 nor UNIX "
		"find style."
		)
	ENDIF(NOT wxWidgets_FIND_QUIETLY)
ENDIF(wxWidgets_FIND_STYLE STREQUAL "win32")

# Debug output:
DBG_MSG("wxWidgets_FOUND           : ${wxWidgets_FOUND}")
DBG_MSG("wxWidgets_INCLUDE_DIRS    : ${wxWidgets_INCLUDE_DIRS}")
DBG_MSG("wxWidgets_LIBRARY_DIRS    : ${wxWidgets_LIBRARY_DIRS}")
DBG_MSG("wxWidgets_LIBRARIES       : ${wxWidgets_LIBRARIES}")
DBG_MSG("wxWidgets_CXX_FLAGS       : ${wxWidgets_CXX_FLAGS}")
DBG_MSG("wxWidgets_USE_FILE        : ${wxWidgets_USE_FILE}")

#=====================================================================
#=====================================================================
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(wxWidgets DEFAULT_MSG wxWidgets_FOUND)
# Maintain consistency with all other variables.
SET(wxWidgets_FOUND ${WXWIDGETS_FOUND})

#=====================================================================
# Macros for use in wxWidgets apps.
# - This module will not fail to find wxWidgets based on the code
#   below. Hence, it's required to check for validity of:
#
# wxWidgets_wxrc_EXECUTABLE
#=====================================================================

# Resource file compiler.
FIND_PROGRAM(wxWidgets_wxrc_EXECUTABLE wxrc
	${wxWidgets_ROOT_DIR}/utils/wxrc/vc_msw
	)

# 
# WX_SPLIT_ARGUMENTS_ON(<keyword> <left> <right> <arg1> <arg2> ...)
# 
# Sets <left> and <right> to contain arguments to the left and right,
# respectively, of <keyword>.
# 
# Example usage:
#  FUNCTION(WXWIDGETS_ADD_RESOURCES outfiles)
#    WX_SPLIT_ARGUMENTS_ON(OPTIONS wxrc_files wxrc_options ${ARGN})
#    ...
#  ENDFUNCTION(WXWIDGETS_ADD_RESOURCES)
#
#  WXWIDGETS_ADD_RESOURCES(sources ${xrc_files} OPTIONS -e -o file.C)
# 
# NOTE: This is a generic piece of code that should be renamed to
# SPLIT_ARGUMENTS_ON and put in a file serving the same purpose as
# FindPackageStandardArgs.cmake. At the time of this writing
# FindQt4.cmake has a QT4_EXTRACT_OPTIONS, which I basically copied
# here a bit more generalized. So, there are already two find modules
# using this approach.
#
FUNCTION(WX_SPLIT_ARGUMENTS_ON _keyword _leftvar _rightvar)
	# FIXME: Document that the input variables will be cleared.
	#LIST(APPEND ${_leftvar}  "")
	#LIST(APPEND ${_rightvar} "")
	SET(${_leftvar}  "")
	SET(${_rightvar} "")

	SET(_doing_right FALSE)
	FOREACH(element ${ARGN})
		IF("${element}" STREQUAL "${_keyword}")
			SET(_doing_right TRUE)
		ELSE("${element}" STREQUAL "${_keyword}")
			IF(_doing_right)
				LIST(APPEND ${_rightvar} "${element}")
			ELSE(_doing_right)
				LIST(APPEND ${_leftvar} "${element}")
			ENDIF(_doing_right)
		ENDIF("${element}" STREQUAL "${_keyword}")
	ENDFOREACH(element)

	SET(${_leftvar}  ${${_leftvar}}  PARENT_SCOPE)
	SET(${_rightvar} ${${_rightvar}} PARENT_SCOPE)
ENDFUNCTION(WX_SPLIT_ARGUMENTS_ON)

#
# WX_GET_DEPENDENCIES_FROM_XML(
#   <depends>
#   <match_pattern>
#   <clean_pattern>
#   <xml_contents>
#   <depends_path>
#   )
#
# FIXME: Add documentation here...
#
FUNCTION(WX_GET_DEPENDENCIES_FROM_XML
	_depends
	_match_patt
	_clean_patt
	_xml_contents
	_depends_path
	)

	STRING(REGEX MATCHALL
		${_match_patt}
		dep_file_list
		"${${_xml_contents}}"
		)
	FOREACH(dep_file ${dep_file_list})
		STRING(REGEX REPLACE ${_clean_patt} "" dep_file "${dep_file}")

		# make the file have an absolute path
		IF(NOT IS_ABSOLUTE "${dep_file}")
			SET(dep_file "${${_depends_path}}/${dep_file}")
		ENDIF(NOT IS_ABSOLUTE "${dep_file}")

		# append file to dependency list
		LIST(APPEND ${_depends} "${dep_file}")
	ENDFOREACH(dep_file)

	SET(${_depends} ${${_depends}} PARENT_SCOPE)
ENDFUNCTION(WX_GET_DEPENDENCIES_FROM_XML)

# # 
# # WXWIDGETS_ADD_RESOURCES(<sources> <xrc_files>
# #                         OPTIONS <options> [NO_CPP_CODE])
# # 
# # Adds a custom command for resource file compilation of the
# # <xrc_files> and appends the output files to <sources>.
# # 
# # Example usages:
# #   WXWIDGETS_ADD_RESOURCES(sources xrc/main_frame.xrc)
# #   WXWIDGETS_ADD_RESOURCES(sources ${xrc_files} OPTIONS -e -o altname.cxx)
# #
FUNCTION(WXWIDGETS_ADD_RESOURCES _outfiles)
	WX_SPLIT_ARGUMENTS_ON(OPTIONS rc_file_list rc_options ${ARGN})

	# Parse files for dependencies.
	SET(rc_file_list_abs "")
	SET(rc_depends "")
	FOREACH(rc_file ${rc_file_list})
		GET_FILENAME_COMPONENT(depends_path ${rc_file} PATH)

		GET_FILENAME_COMPONENT(rc_file_abs ${rc_file} ABSOLUTE)
		LIST(APPEND rc_file_list_abs "${rc_file_abs}")

		# All files have absolute paths or paths relative to the location
		# of the rc file.
		FILE(READ "${rc_file_abs}" rc_file_contents)

		# get bitmap/bitmap2 files
		WX_GET_DEPENDENCIES_FROM_XML(
			rc_depends
			"<bitmap[^<]+"
			"^<bitmap[^>]*>"
			rc_file_contents
			depends_path
			)

		# get url files
		WX_GET_DEPENDENCIES_FROM_XML(
			rc_depends
			"<url[^<]+"
			"^<url[^>]*>"
			rc_file_contents
			depends_path
			)

		# get wxIcon files
		WX_GET_DEPENDENCIES_FROM_XML(
			rc_depends
			"<object[^>]*class=\"wxIcon\"[^<]+"
			"^<object[^>]*>"
			rc_file_contents
			depends_path
			)
	ENDFOREACH(rc_file)

	#
	# Parse options.
	# 
	# If NO_CPP_CODE option specified, then produce .xrs file rather
	# than a .cpp file (i.e., don't add the default --cpp-code option).
	LIST(FIND rc_options NO_CPP_CODE index)
	IF(index EQUAL -1)
		LIST(APPEND rc_options --cpp-code)
		# wxrc's default output filename for cpp code.
		SET(outfile resource.cpp)
	ELSE(index EQUAL -1)
		LIST(REMOVE_AT rc_options ${index})
		# wxrc's default output filename for xrs file.
		SET(outfile resource.xrs)
	ENDIF(index EQUAL -1)

	# Get output name for use in ADD_CUSTOM_COMMAND.
	# - short option scanning
	LIST(FIND rc_options -o index)
	IF(NOT index EQUAL -1)
		MATH(EXPR filename_index "${index} + 1")
		LIST(GET rc_options ${filename_index} outfile)
		#LIST(REMOVE_AT rc_options ${index} ${filename_index})
	ENDIF(NOT index EQUAL -1)
	# - long option scanning
	STRING(REGEX MATCH "--output=[^;]*" outfile_opt "${rc_options}")
	IF(outfile_opt)
		STRING(REPLACE "--output=" "" outfile "${outfile_opt}")
	ENDIF(outfile_opt)
	#STRING(REGEX REPLACE "--output=[^;]*;?" "" rc_options "${rc_options}")
	#STRING(REGEX REPLACE ";$" "" rc_options "${rc_options}")
	
	IF(NOT IS_ABSOLUTE "${outfile}")
		SET(outfile "${CMAKE_CURRENT_BINARY_DIR}/${outfile}")
	ENDIF(NOT IS_ABSOLUTE "${outfile}")
	ADD_CUSTOM_COMMAND(
		OUTPUT "${outfile}"
		COMMAND ${wxWidgets_wxrc_EXECUTABLE} ${rc_options} ${rc_file_list_abs}
		DEPENDS ${rc_file_list_abs} ${rc_depends}
		)

	# Add generated header to output file list.
	LIST(FIND rc_options -e short_index)
	LIST(FIND rc_options --extra-cpp-code long_index)
	IF(NOT short_index EQUAL -1 OR NOT long_index EQUAL -1)
		GET_FILENAME_COMPONENT(outfile_ext ${outfile} EXT)
		STRING(REPLACE "${outfile_ext}" ".h" outfile_header "${outfile}")
		LIST(APPEND ${_outfiles} "${outfile_header}")
		SET_SOURCE_FILES_PROPERTIES(
		"${outfile_header}" PROPERTIES GENERATED TRUE
		)
	ENDIF(NOT short_index EQUAL -1 OR NOT long_index EQUAL -1)

	# Add generated file to output file list.
	LIST(APPEND ${_outfiles} "${outfile}")

	SET(${_outfiles} ${${_outfiles}} PARENT_SCOPE)
ENDFUNCTION(WXWIDGETS_ADD_RESOURCES)

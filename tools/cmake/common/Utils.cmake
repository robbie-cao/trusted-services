#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#[===[.rst:
Misc utilities
--------------
#]===]

include_guard(DIRECTORY)

#[===[.rst:
.. cmake:command:: check_args

  .. code-block:: cmake

    check_args(func_name REQ_ARG1 REQ_ARG2)

  Helper macro for argument checking in functions. First argument *func_name* is
  the name of the function, other arguments are the names of the required
  arguments to that function. The macro iterates through the list, and prints
  and error message if not all arguments are defined.

#]===]
macro(check_args)
	set(_argv "${ARGV}")
	list(SUBLIST _argv 0 1 _func)
	list(SUBLIST _argv 1 -1 _args)
	foreach(_arg IN LISTS _args)
		if (NOT DEFINED _MY_PARAMS_${_arg})
			message(FATAL_ERROR "${_func}(): mandatory parameter '${_arg}' missing.")
		endif()
	endforeach()
endmacro()

# Verify MSYS environment.
function(ts_verify_build_env)
    if (WIN32)
        #On MSYS2 64 bit builds do not work. Verify environment.
        execute_process(COMMAND uname -s
                        OUTPUT_VARIABLE _os_name)
        #If uname is present we assume MSYS environment and the os name must
        #contain MING32.
        if(_os_name STREQUAL "" AND NOT _os_name MATCHES ".*MINGW32.*")
            message(FATAL_ERROR "This seems to be a 64 bit MINGW shell, which has issues. Please run the 32bit version.")
        endif()
    endif()
endFunction()

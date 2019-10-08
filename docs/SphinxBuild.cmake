#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Minimal cmake script for running sphinx. Use as:
# cmake -P SphinxBuild.cmake

# Inputs:
#   SPHINXOPTS : extra options for sphinx

set(SPHINXBUILD "sphinx-build" CACHE PATH "Location of sphinx-build executable.")
set(SPHNIX_BUILDDIR "_build" CACHE PATH "Directory to place sphinx outpot to.")

exec_program(${SPHINXBUILD} ./
             ARGS -M html ${CMAKE_CURRENT_LIST_DIR} ${SPHNIX_BUILDDIR} ${SPHINXOPTS}
            )

# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

.SUFFIXES:
.SUFFIXES: .j2

# Disable parallel execution for this makefile.
.NOTPARALLEL:

TEMPLATES:=$(wildcard *.j2)
OUTPUTS:=$(TEMPLATES:%.j2=%)

all:
	make config
	bash run.sh

config: ${OUTPUTS}

list:
	make config
	bash run.sh help

# run a command of the generated script
r-%:
	make config
	bash run.sh "$*"

define help_msg
*************************************************************
Available targets:
  all          - generate and run test script
  config       - run script generation only
  clean        - remove intermediate files
  list         - run config and list test cases
  p-<variable> - print value of make variable (for debugging)
  r-<test>     - run a command of the generated script. Allow
                 executing a single test case. Use
                    make r-help
                 to get the list of tests.
*************************************************************
endef

export help_msg
help:
	@echo "$$help_msg"

# remove intermediate output
clean:
ifneq ($(wildcard ./run.sh),)
	bash run.sh clean
endif
	-rm -rf ${OUTPUTS}

# Convert template to shell script
%.sh : %.sh.j2 test_data.yaml
	yasha -v test_data.yaml $<

# print variable value (i.e. make p-TEMPLATES)
p-%:
	@echo $*=${$*}

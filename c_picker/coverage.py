#!/usr/bin/env python3
# Copyright (c) 2020-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

"""
This module can map the coverage of c-picker generated files to the original source files.
"""

import argparse
import json
import os
import re
import sys

class MappingDescriptor:
    """ Storage class for file-line number data """

    def __init__(self, file=None, line=None):
        self.descriptor = {"file": file, "line": line}

    def get_file(self):
        """ Queries file name """
        return self.descriptor["file"]

    def get_line(self):
        """ Queries line number """
        return self.descriptor["line"]

    @staticmethod
    def serialize(mapping_descriptor):
        """ Serializes the descriptor into a C comment containing JSON. """
        return "/* C-PICKER-MAPPING " + json.dumps(mapping_descriptor.descriptor) + " */"

    @staticmethod
    def deserialize(line):
        """
        Deserializes the descriptor from a C comment containing JSON.
        It returns None if the line is not matching the required pattern.
        """
        match = re.match(r"/\* C-PICKER-MAPPING ({.*}) \*/", line)
        if not match:
            return None

        mapping_descriptor = MappingDescriptor()
        mapping_descriptor.descriptor = json.loads(match.group(1))
        return mapping_descriptor

class CoverageMapper:
    """ The class maps the coverage of the c-picker generated source files to the original ones. """

    def __init__(self):
        self.mapping_path = None
        self.output = sys.stdout

        self.test_name = None
        self.mapping_enabled = False
        self.mapping_descriptors = {}
        self.mapped_source_file = None

    def read_mapping_descriptors(self, filename):
        """ Reads the mapping descriptors from the c-picker generated file. """
        self.mapping_enabled = True

        with open(filename, "r") as source_file:
            source_lines = source_file.read().split("\n")
            source_line_index = 1

            for source_line in source_lines:
                mapping_descriptor = MappingDescriptor.deserialize(source_line)

                if mapping_descriptor:
                    # +1: the elements start at the following line after the descriptor comment
                    self.mapping_descriptors[source_line_index + 1] = mapping_descriptor
                source_line_index += 1

    def clear_mapping_descriptors(self):
        """ Resets the mapping descriptor database. """
        self.mapping_enabled = False
        self.mapping_descriptors = {}
        self.mapped_source_file = None

    def find_mapping(self, line_number):
        """ Find the mapping descriptor of a line number and also returns the mapped line. """
        mapping_descriptor_index = 0
        for i in self.mapping_descriptors:
            if i > line_number:
                break
            mapping_descriptor_index = i

        if not mapping_descriptor_index:
            raise Exception("Invalid mapping for line %d" % line_number)

        mapping_descriptor = self.mapping_descriptors[mapping_descriptor_index]
        mapped_line = line_number - mapping_descriptor_index + mapping_descriptor.get_line()
        return mapping_descriptor, mapped_line

    def output_line(self, line):
        """ Outputs a single line to the output """
        self.output.write(line + "\n")

    def process_line(self, trace_line):
        """
        The function processes a single line of the trace file and maintains the internal state of
        the state matchine.
        """
        if not trace_line:
            return

        if trace_line == "end_of_record":
            # End of record, exit mapping mode
            self.clear_mapping_descriptors()
            self.output_line(trace_line)
            return

        command, params = trace_line.split(":", 1)
        if command == "TN":
            # Test name TN:<test name>
            self.test_name = params
        elif command == "SF" and params.startswith(self.mapping_path):
            # Source file SF:<absolute path to the source file>
            # Matching source file, switch into mapping mode
            self.read_mapping_descriptors(params)
            return

        if self.mapping_enabled and (command in ("FN", "BRDA", "DA")):
            # Function        FN:<line number of function start>,<function name>
            # Branch coverage BRDA:<line number>,<block number>,<branch number>,<taken>
            # Line coverage   DA:<line number>,<execution count>[,<checksum>]
            line_number, remaining_params = params.split(",", 1)
            mapping_descriptor, mapped_line_number = self.find_mapping(int(line_number))

            if mapping_descriptor.get_file() != self.mapped_source_file:
                # Change in the name of the mapped source file, starting new record
                if self.mapped_source_file is not None:
                    # No need for this part if it was the first mapped file
                    # because TN has been alread printed
                    self.output_line("end_of_record")
                    self.output_line("TN:%s" % self.test_name)
                self.mapped_source_file = mapping_descriptor.get_file()
                self.output_line("SF:%s" % self.mapped_source_file)

            self.output_line("%s:%d,%s" % (command, mapped_line_number, remaining_params))
            return

        self.output_line(trace_line)

    def main(self):
        """ Runs coverage mapper configured by the command line arguments  """

        try:
            parser = argparse.ArgumentParser(prog="c-picker-coverage-mapper", description=__doc__)
            parser.add_argument("--input", help="Input trace file", required=True)
            parser.add_argument("--output", help="Input file")
            parser.add_argument("--mapping-path", help="Directory of generated files",
                                required=True)
            args = parser.parse_args()

            output_fp = open(args.output, "w") if args.output else None

            self.output = output_fp if output_fp else sys.stdout
            self.mapping_path = os.path.abspath(args.mapping_path)

            with open(args.input, "r") as tracefile:
                trace_lines = tracefile.read().split("\n")
                for trace_line in trace_lines:
                    self.process_line(trace_line)

            if output_fp:
                output_fp.close()

            return 0
        except FileNotFoundError as exception:
            print("File not found: %s" % str(exception), file=sys.stderr)
        except ValueError as exception:
            print("Invalid format: %s" % str(exception), file=sys.stderr)
        except Exception as exception: # pylint: disable=broad-except
            print("Exception: %s" % exception, file=sys.stderr)

        return 1

def main():
    """ Command line main function """
    coverage_mapper = CoverageMapper()
    result = coverage_mapper.main()
    sys.exit(result)

if __name__ == "__main__":
    main()

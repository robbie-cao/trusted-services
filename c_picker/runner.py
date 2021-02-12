#!/usr/bin/env python3
# Copyright (c) 2020-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

"""
This module wraps the c-picker functionality into a command line interface.
"""

import argparse
import json
import sys
import yaml

from c_picker import __version__ as CPICKER_VERSION
from c_picker.picker import CPicker
from c_picker.picker import __doc__ as c_picker_doc

class CPickerRunner:
    """ Command line wrapper for CPicker """

    def __init__(self):
        self.root = ""
        self.args = []
        self.options = []

        self.parser = argparse.ArgumentParser(prog="c-picker", description=c_picker_doc)

        self.parser.add_argument("--root", help="Root source directory")
        self.parser.add_argument("--config", help="Configuration file (.json|.yml)", required=True)
        self.parser.add_argument("--output", help="Output file")
        self.parser.add_argument("--print-dependencies", help="Print dependencies",
                                 action="store_true")
        self.parser.add_argument("--version", action="version", version=CPICKER_VERSION)
        self.parser.add_argument("--args", help="clang arguments", nargs=argparse.REMAINDER)

    @staticmethod
    def error(message):
        """ Print error message to stderr """
        print("c-picker error\n" + message, file=sys.stderr)

    def generate_element_descriptors(self, elements):
        """ Converts the structure of the config file to a list of ElementDescriptors """
        element_descriptors = []

        for element in elements:
            file_name = self.root + element["file"]
            element_name = element["name"] if "name" in element else None
            element_type = CPicker.Type[element["type"]]
            element_args = self.args + element.get("args", [])
            element_options = [
                CPicker.Option[opt]
                for opt in self.options + element.get("options", [])]

            descriptor = CPicker.ElementDescriptor(file_name, element_type, element_name)
            descriptor.set_args(element_args)
            descriptor.set_options(element_options)

            element_descriptors.append(descriptor)

        return element_descriptors

    @staticmethod
    def create_config(args):
        """ Create a configuration object from the command line arguments and config files. """
        try:
            if args.config.endswith(".json"):
                with open(args.config) as json_file:
                    return json.load(json_file)
            elif args.config.endswith(".yml"):
                with open(args.config) as yml_file:
                    return yaml.safe_load(yml_file)
            else:
                raise Exception("Invalid configuration file %s" % args.config)
        except json.decoder.JSONDecodeError as exception:
            raise Exception("Invalid JSON format: " + str(exception))
        except yaml.YAMLError as exception:
            raise Exception("Invalid YAML format: " + str(exception))
        except FileNotFoundError as exception:
            raise Exception("File not found: " + str(exception))

    def main(self):
        """ Runs CPicker configured by the command line arguments  """
        try:
            args = self.parser.parse_args()
            config = self.create_config(args)

            output_fp = open(args.output, "w") if args.output else None

            self.root = args.root + "/" if args.root else ""
            self.args = args.args if args.args else [] + config.get("args", [])
            self.options = config.get("options", [])

            elements = self.generate_element_descriptors(config.get("elements", []))

            c_picker = CPicker(output_fp, args.print_dependencies)
            c_picker.process(elements)

            if output_fp:
                output_fp.close()

            return 0
        except KeyError as exception:
            self.error("Key error: " + str(exception))
        except Exception as exception: # pylint: disable=broad-except
            self.error("Error: " + str(exception))

        return 1

def main():
    """ Command line main function """
    c_picker_runner = CPickerRunner()
    result = c_picker_runner.main()
    sys.exit(result)

if __name__ == '__main__':
    main()

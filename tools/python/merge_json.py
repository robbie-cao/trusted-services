#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright (c) 2022-2023, Arm Limited. All rights reserved.

"""Merge json files and print the result to STDOUT.

Source files are specified with a list of file names. Any name in the list can
be a glob pattern. Glob patterns act if the returned list of file names were
passed instead. The list returned by glob is not sorted. All files are
processed in the order being found in the argument list.

Note: do not forget to quote globing patterns when running the tool from a
      shell.
"""

import argparse
import errno
import glob
import json
import logging
import os.path
import sys

# initialize logger
logging.getLogger('merge_json')
logging.basicConfig(level=logging.ERROR)


def parse_arguments(args):
    parser = argparse.ArgumentParser(
                prog=os.path.basename(args[0]),
                description=__doc__,
                formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
                "-e", "--exclude",
                default=None,
                metavar="<exclude pattern>",
                help="Exclude files matching this pattern.")
    parser.add_argument(
                "-o", "--output_file",
                default=None,
                metavar="<path to output file>",
                help="Write result to this file instead of STDOUT")
    parser.add_argument(
                "-v",
                action='append_const', const='v',
                metavar="<verbosity level>",
                help="Set the amount of information printed to STDERR." +
                     " Passing more times gives more info.")
    parser.add_argument(
                "-i", "--ignore-missing",
                dest="ignore_missing",
                action='store_const', const=True, default=False,
                help="Ignore missing source files or source globs returning" +
                     " empty result.")
    parser.add_argument(
                'source_list',
                nargs="+",
                metavar="<source file>",
                help="List of source files (file name can be glob pattern).")
    parsed_args = parser.parse_args(args[1:])
    # Count -v arguments to logging level
    if parsed_args.v:
        llv = len(parsed_args.v)
        if llv > 3:
            llv = 3
    else:
        llv = 0
    parsed_args.log_level = [logging.ERROR, logging.WARNING, logging.INFO,
                             logging.DEBUG][llv]
    return (parsed_args)


def merge_files(parsed_args):
    logger = logging.getLogger('merge_json')

    logger.info(
        "Merging " + str(parsed_args.source_list) + " to " +
        (parsed_args.output_file if parsed_args.output_file else "STDOUT"))

    result = {}
    exclude_list = None

    if parsed_args.exclude:
        exclude_list = glob.glob(parsed_args.exclude, recursive=True)
        if exclude_list:
            logger.debug("Excluding files: %s" % exclude_list)
        else:
            logger.warning("Exclude pattern matches no files.")

    for pattern in parsed_args.source_list:
        file_list = glob.glob(pattern, recursive=True)
        logger.debug("Globing " + pattern + " = " + str(file_list))
        if not file_list:
            logger.error("Pattern \"%s\" does not match any file" % pattern)
            if not parsed_args.ignore_missing:
                raise (FileNotFoundError(
                        errno.ENOENT,
                        "Pattern does not match any file",
                        pattern))

        for file in list(file_list):
            if exclude_list and file in list(exclude_list):
                logger.debug("excluding file " + file)
                continue
            logger.debug("Reding source file " + file)
            with open(file, "rt", encoding="utf8", errors="strict") as f:
                result.update(json.load(f))

    if parsed_args.output_file is not None:
        path = os.path.dirname(parsed_args.output_file)
        if path:
            os.makedirs(path, exist_ok=True)
        with open(parsed_args.output_file, "w", encoding="utf8") as f:
            json.dump(result, f, indent=4)
    else:
        print(json.dumps(result, indent=4))


if __name__ == "__main__":
    parsed_args = parse_arguments(sys.argv)
    logger = logging.getLogger('merge_json')
    logger.setLevel(parsed_args.log_level)
    merge_files(parsed_args)

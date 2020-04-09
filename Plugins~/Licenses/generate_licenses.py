#!/usr/bin/env python

import sys

import argparse
import html
import json
import logging
import os
import re
import subprocess


LIB_TO_LICENSES_DICT = {
    'Autodesk': ['autodesk/LICENSE.txt'],
    'Blender': ['blender/LICENSE.txt'],
    'glew': ['glew/LICENSE.txt'],
    'pybind11': ['pybind11/LICENSE'],
    'python35': ['python35/LICENSE'],
    'python36': ['python36/LICENSE'],
    'python37': ['python37/LICENSE'],
    'WTL': ['WTL/MS-PL.txt'],
    'MeshSync': ['MeshSync/LICENSE.md'],
    'ispc': ['ispc/LICENSE.txt'],
    'Poco': ['Poco/LICENSE.txt'],
    'zstd': ['zstd/LICENSE'],       
}

SCRIPT_DIR = os.path.dirname(os.path.realpath(sys.argv[0]))

THIRD_PARTY_LIB_REGEX = r'^.*/third_party/([\w\-+]+).*$'

class LicenseBuilder(object):

  @staticmethod
  def _ParseLibrary(dep):
    """
    Returns a regex match containing library name after third_party

    Input one of:
    //a/b/third_party/libname:c
    //a/b/third_party/libname:c(//d/e/f:g)
    //a/b/third_party/libname/c:d(//e/f/g:h)

    Outputs match with libname in group 1 or None if this is not a third_party
    dependency.
    """
    return re.match(THIRD_PARTY_LIB_REGEX, dep)

  @staticmethod
  def _RunGN(buildfile_dir, target):
    cmd = [
      sys.executable,
      os.path.join(find_depot_tools.DEPOT_TOOLS_PATH, 'gn.py'),
      'desc',
      '--all',
      '--format=json',
      os.path.abspath(buildfile_dir),
      target,
    ]
    logging.debug("Running: %r", cmd)
    output_json = subprocess.check_output(cmd, cwd=SCRIPT_DIR)
    logging.debug("Output: %s", output_json)
    return output_json

  @staticmethod
  def _GetThirdPartyLibraries(buildfile_dir, target):
    output = json.loads(LicenseBuilder._RunGN(buildfile_dir, target))
    libraries = set()
    for target in output.values():
      third_party_matches = (
          LicenseBuilder._ParseLibrary(dep) for dep in target['deps'])
      libraries |= set(match.group(1) for match in third_party_matches if match)
    return libraries

  def GenerateLicenseText(self, output_dir):
    # Get a list of third_party libs from gn. For fat libraries we must consider
    # all architectures, hence the multiple buildfile directories.
    third_party_libs = set()

    missing_licenses = third_party_libs - set(LIB_TO_LICENSES_DICT.keys())
    if missing_licenses:
      error_msg = 'Missing licenses: %s' % ', '.join(missing_licenses)
      logging.error(error_msg)
      raise Exception(error_msg)

    license_libs = sorted(LIB_TO_LICENSES_DICT.keys())

    logging.info("List of licenses: %s", ', '.join(license_libs))

    # Generate markdown.
    output_license_file = open(os.path.join(output_dir, 'Third Party Notices.md'), 'w+')
    output_license_file.write('This repository contains third-party software components governed by the license(s) indicated below:\n\n')
    
    for license_lib in license_libs:
      if len(LIB_TO_LICENSES_DICT[license_lib]) == 0:
        logging.info("Skipping compile time dependency: %s", license_lib)
        continue # Compile time dependency

      output_license_file.write('# %s\n' % license_lib)
      output_license_file.write('```\n')
      for path in LIB_TO_LICENSES_DICT[license_lib]:
        license_path = os.path.join(SCRIPT_DIR, path)
        with open(license_path, encoding="utf8") as license_file:
          license_text = html.escape(license_file.read(), quote=True)
          output_license_file.write(license_text)
          output_license_file.write('\n')
      output_license_file.write('```\n\n')

    output_license_file.close()


#example: generate_licenses.py ..
def main():
    parser = argparse.ArgumentParser(description='Generate Third Party Notices.md')
    parser.add_argument('--verbose', action='store_true', default=False, help='Debug logging.')
    parser.add_argument('output_dir', help='Directory to output Third Party Notices.md to.')
    args = parser.parse_args()

    logging.basicConfig(level=logging.DEBUG if args.verbose else logging.INFO)

    builder = LicenseBuilder()
    builder.GenerateLicenseText(args.output_dir)


if __name__ == '__main__':
  sys.exit(main())

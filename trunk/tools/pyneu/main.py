#!/usr/bin/python


import codecs
import compiler
import consts
import optparse
import os
import scanner
import sys


SYNTAX_ERROR_TEMPLATE = """\
%(filename)s:%(line)i:%(range)s: %(message)s
%(source)s
%(underline)s\
"""


def main(options, args):
  consts.initialize_from(options.consts)
  files = reduce(list.__add__, map(find_source_files, args), [])
  try:
    compiler.compile(files, options.out)
  except scanner.CompilationError, ce:
    report_error(ce)


def report_error(ce):
  pos = ce.position()
  doc = pos.document()
  lineno = doc.get_line_of_offset(pos.start())
  source = codecs.utf_8_encode(doc.get_line(lineno))[0]
  linestart = doc.get_line_start(lineno)
  spaces = " " * (pos.start() - linestart)
  wave = "^" * (pos.end() - pos.start() + 1)
  startcol = pos.start() - linestart + 1
  endcol = pos.end() - linestart + 1
  if startcol == endcol: r = str(startcol)
  else: r = "%i-%i" % (startcol, endcol)
  print SYNTAX_ERROR_TEMPLATE % {
    'filename': doc.name,
    'line': lineno + 1,
    'range': r,
    'message': ce.message(),
    'source': source,
    'underline': spaces + wave
  }


def ignore_file(name):
  return os.path.basename(name).startswith('.')


def find_source_files(location):
  """Returns a list of all the neutrino source files in a given
  directory"""
  result = []
  if os.path.isdir(location):
    for (dirpath, dirnames, filenames) in os.walk(location):
      # Prune ignored directories.  Use a list comprehension to avoid
      # iterating through the list itself while concurrently modifying
      # it
      for ignored in [f for f in dirnames if ignore_file(f)]:
        dirnames.remove(ignored)
      for f in filenames:
        if f.endswith('.n') and not ignore_file(f):
          result.append(os.path.join(dirpath, f))
  else:
    result.append(location)
  return result


def build_option_parser():
  result = optparse.OptionParser()
  result.add_option('-c', '--consts')
  result.add_option('-o', '--out')
  return result


def validate_options(options):
  if not options.consts:
    return "A constants file must be specified"
  if not os.path.exists(options.consts):
    return "Constants file '%s' doesn't exist" % options.consts
  if not options.out:
    return "An output file must be specified"
  return None


if __name__ == '__main__':
  parse = build_option_parser()
  (options, args) = parse.parse_args()
  error_msg = validate_options(options)
  if error_msg:
    print error_msg
    parse.print_help()
    sys.exit(1)
  sys.exit(main(options, args))

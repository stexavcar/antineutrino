#!/usr/bin/env python

import optparse
import sys

OPTIONS = [
  ('mode', {
    'values': ['debug', 'release'],
    'default': 'release',
    'always_in_variant': True
  }),
  ('wordsize', {
    'values': ['32', '64'],
    'default': '32'
  }),
  ('toolchain', {
    'values': ['msvc', 'gcc', 'guess'],
    'default': 'guess'
  })
]

def main():
  opts = optparse.OptionParser()
  for (key, option) in OPTIONS:
    opts.add_option('--%s' % key)
    print key, option
  if len(sys.argv) < 2:
    print_usage()
    return 1
  return 0

if __name__ == '__main__':
  sys.exit(main())

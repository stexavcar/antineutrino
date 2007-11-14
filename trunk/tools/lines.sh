#!/bin/sh

find . -not '(' -name .svn -prune ')' -and '(' -name \*.h -or -name \*.cc ')' | xargs wc

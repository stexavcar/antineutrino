#!/bin/sh

ARGC=0
declare -a ARGV
HPROF="no"
CLASSPATH=bin/classes
while [ -n "$1" ]; do
  case $1 in
    --hprof) HPROF="yes";;
    --classpath=*) CLASSPATH=${1#--classpath=};;
    *) ARGV[$ARGC]="$1"; ARGC=$(($ARGC + 1));
  esac
  shift
done

function set_profiler_flags {
  if [ $HPROF = "yes" ]; then
    PROFILER_FLAGS="-Xrunhprof:cpu=samples,file=$1.hprof.txt"
  else
    PROFILER_FLAGS=""
  fi
}

JAVA_FLAGS="-ea -cp $CLASSPATH"

function run_main {
  if [ ! -d $CLASSPATH ]; then
    echo "Classpath doesn't exist.  Did you 'ant compile' before running this?"
    exit 1
  fi
  set_profiler_flags $1
  java $JAVA_FLAGS $PROFILER_FLAGS $1 ${ARGV[*]}
}

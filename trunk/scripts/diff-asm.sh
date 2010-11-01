#!/bin/sh

OBJ=$1
EXP=$2

otool -tVX $OBJ | grep -v indirect | tr '\011' ' ' | sed "s/^ //g" > $OBJ.asm \
  && diff -c $EXP $OBJ.asm

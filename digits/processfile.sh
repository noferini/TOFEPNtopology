#!/bin/bash

export thread=$(echo $1 | sed 's/^0*//')
export run=$(echo $2 | sed 's/^0*//')

if [ "x$thread" == "x" ]; then
export thread=0
fi

if [ "x$run" == "x" ]; then
export run=0
fi

export dir=$3

module load O2PDPSuite

root -b -q -l processDigitsforCosmics.C\(\"$dir\",$thread,$run\)

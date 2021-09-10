#!/bin/bash

# requirements: processDigitsforCosmics.C, processfile.sh and o2sim_geometry.root in working dir
# output written in working dir

# ./processcosmics.sh DIR_INPUT
# DIR_INPUT where digits are

dirIn=$1

ls $dirIn|grep tofdigits|awk -F"_" '{print $2,$3}'|awk -F"." '{print $1}' >lista

cat lista|grep 0|awk '{print "./processfile.sh",$1,$2,"'$dirIn'"}'

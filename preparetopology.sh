#!/bin/bash
module load DataDistribution ODC QualityControl O2

export Nepn=$2
if [ "x${Nepn}" == "x" ]; then
  export Nepn=5
fi

export calibration_node=$3
if [ "x${Nepn}" == "x" ]; then
  export calibration_node="epn003:30453"
fi


./$1.sh |grep -v INFO >$1.xml

odc-topo --dpl $1.xml --ndpl ${Nepn} --dd /home/epn/odc/dd-data.xml --ndd ${Nepn} -o $1-${Nepn}epn.xml

rm $1.xml

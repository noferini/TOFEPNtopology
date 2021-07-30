#!/bin/bash
module load DataDistribution ODC QualityControl O2

export Nepn=$2
if [ "x${Nepn}" == "x" ]; then
  export Nepn=5
fi

./$1.sh |grep -v INFO >$1.xml

odc-topo --dpl $1.xml --ndpl ${Nepn} --dd /home/epn/odc/dd-data.xml --ndd ${Nepn} -o $1-${Nepn}epn.xml

rm $1.xml

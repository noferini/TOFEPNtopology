#!/usr/bin/env bash
export calibration_node=$1
set -u

module load O2PDPSuite

if [ "x${calibration_node}" == "x" ]; then
  export calibration_node="epn003-ib:30453"
fi

# DO NOT MODIFY
OUT_CHANNEL="name=downstream,method=connect,address=tcp://${calibration_node},type=push,transport=zeromq,rateLogging=1"

echo ${OUT_CHANNEL}

## TOF-EPN-LOCAL
SHMSIZE=400000000
#SHMSIZE=$(( 64 << 30 )) # 64 GiB
SEVERITY="info"
GRP_PATH=""
#GRP_PATH="--configKeyValues NameConf.mDirGRP=/home/epn/odc/files;NameConf.mDirGeom=/home/epn/odc/files"
ARGS_ALL="--session default --severity $SEVERITY --shm-segment-size $SHMSIZE -b"

PROXY_OUTSPEC="calclus:TOF/INFOCALCLUS;cosmics:TOF/INFOCOSMICS"

o2-tof-cluscal-reader-workflow --rate 440 ${ARGS_ALL} --cosmics --tof-calclus-infile tofclusCalInfoOr.root \
| o2-dpl-output-proxy ${ARGS_ALL} --channel-config ${OUT_CHANNEL} --dataspec ${PROXY_OUTSPEC} \
| o2-dpl-run ${ARGS_ALL} #--dds # option instead iof run to export DDS xml file

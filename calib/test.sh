#!/usr/bin/env bash
export calibration_node=$1
set -u

module load O2PDPSuite

if [ "x${calibration_node}" == "x" ]; then
  export calibration_node="epn003:30453"
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

PROXY_OUTSPEC="dd:FLP/DISTSUBTIMEFRAME;calclus:TOF/INFOCALCLUS;cosmics:TOF/INFOCOSMICS;trkcos:TOF/INFOTRACKCOS;trksiz:TOF/INFOTRACKSIZE"

o2-raw-file-reader-workflow ${ARGS_ALL} --input-conf TOFraw.cfg \
| o2-tof-compressor ${ARGS_ALL} \
| o2-tof-reco-workflow --input-type raw --output-type clusters ${GRP_PATH} ${ARGS_ALL} --disable-root-output --calib-cluster --cluster-time-window 5000 --cosmics \
| o2-dpl-output-proxy ${ARGS_ALL} --channel-config ${OUT_CHANNEL} --dataspec ${PROXY_OUTSPEC} \
| o2-dpl-run ${ARGS_ALL} #--dds # option instead iof run to export DDS xml file

#!/usr/bin/env bash
export calibration_port=$1
set -u

module load QualityControl

if [ "x${calibration_port}" == "x" ]; then
  export calibration_port="30453"
fi

export calibration_node="*:$calibration_port"

# DO NOT MODIFY
IN_CHANNEL="name=readout-proxy,method=bind,address=tcp://${calibration_node},type=pull,transport=zeromq,rateLogging=1"


## TOF-EPN-LOCAL
#SHMSIZE=400000000
SHMSIZE=$(( 64 << 30 )) # 64 GiB
SEVERITY="info"
#GRP_PATH=""
GRP_PATH="--configKeyValues NameConf.mDirGRP=/home/epn/odc/files;NameConf.mDirGeom=/home/epn/odc/files"
ARGS_ALL="--session default --severity $SEVERITY --shm-segment-size $SHMSIZE -b"

PROXY_INSPEC="dd:FLP/DISTSUBTIMEFRAME/0;calclus:TOF/INFOCALCLUS/0;cosmics:TOF/INFOCOSMICS/0"

# clean dir from previous runs
rm tofclusCalInfo.root

o2-dpl-raw-proxy ${ARGS_ALL} --dataspec ${PROXY_INSPEC} --channel-config ${IN_CHANNEL} \
| o2-calibration-tof-calib-workflow --cosmics --do-channel-offset --min-entries 50 ${ARGS_ALL} \
| o2-calibration-ccdb-populator-workflow ${ARGS_ALL} \
| o2-tof-cluster-calib-workflow ${ARGS_ALL} \
| o2-qc ${ARGS_ALL} --config json://${PWD}/tofcosmics.json \
| o2-dpl-run ${ARGS_ALL} # --dds # option instead iof run to export DDS xml file

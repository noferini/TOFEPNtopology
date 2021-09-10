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
SHMSIZE=$(( 64 << 32 )) # 64 GiB
SEVERITY="info"
#GRP_PATH=""
GRP_PATH="--configKeyValues NameConf.mDirGRP=/home/epn/odc/files;NameConf.mDirGeom=/home/epn/odc/files"
ARGS_ALL="--session default --severity $SEVERITY --shm-segment-size $SHMSIZE -b"

PROXY_INSPEC="dd:FLP/DISTSUBTIMEFRAME/0;dig:TOF/DIGITS/0;head:TOF/DIGITHEADER/0;row:TOF/READOUTWINDOW/0;patt:TOF/PATTERNS/0"

# clean dir from previous runs
rm tofdigit*

o2-dpl-raw-proxy ${ARGS_ALL} --dataspec ${PROXY_INSPEC} --channel-config ${IN_CHANNEL} \
| o2-tof-digit-writer-workflow ${ARGS_ALL}  --ntf 2640 \
--pipeline "tof-digit-splitter-writer:16" \
| o2-dpl-run ${ARGS_ALL} # --dds # option instead iof run to export DDS xml file

#| o2-qc ${ARGS_ALL} --config ${QUALITYCONTROL_ROOT}/etc/tofcosmics.json \

chmod a+w tofdigits*.root
chmod g+w tofdigits*.root
chmod a+w dpl-config.json
chmod g+w dpl-config.json

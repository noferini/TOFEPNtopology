#!/usr/bin/env bash
export calibration_port=$1
set -u

module load O2

if [ "x${calibration_port}" == "x" ]; then
  export calibration_port="30453"
fi

export calibration_node="localhost:$calibration_port"

# DO NOT MODIFY
IN_CHANNEL="name=downstream,method=bind,address=tcp://${calibration_node},type=pull,transport=zeromq,rateLogging=1"


## TOF-EPN-LOCAL
SHMSIZE=$(( 64 << 30 )) # 64 GiB
SEVERITY="info"
GRP_PATH="--configKeyValues NameConf.mDirGRP=/home/epn/odc/files;NameConf.mDirGeom=/home/epn/odc/files"
ARGS_ALL="--session default --severity $SEVERITY --shm-segment-size $SHMSIZE -b"

CTF_DICT="--ctf-dict /home/epn/odc/files/ctf_dictionary.root"

PROXY_INSPEC="calclus:TOF/INFOCALCLUS/0;cosmics:TOF/INFOCOSMICS/0;trkcos:TOF/INFOTRACKCOS/0;trksiz:TOF/INFOTRACKSIZE/0"

# Output directory for the CTF, to write to the current dir., remove `--output-dir  $CTFOUT` from o2-ctf-writer-workflow or set to `CTFOUT=\"""\"`
# The directory must exist
#CTFOUT="/home/epn/odc/tofscripts/debug/out"
CTFOUT="/home/eosbuffer/tofctf"

# clean dir from previous runs
rm tofclusCalInfo.root

o2-dpl-raw-proxy --dataspec ${PROXY_INSPEC} --channel-config ${IN_CHANNEL}"name=readout-proxy,type=pull,method=bind,address=tcp://localhost:30453,rateLogging=1,transport=zeromq" \
| o2-tof-cluster-calib-workflow ${ARGS_ALL} --cosmics \
| o2-dpl-run ${ARGS_ALL} # --dds # option instead iof run to export DDS xml file

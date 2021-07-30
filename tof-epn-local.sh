#!/usr/bin/env bash
set -u

# DO NOT MODIFY
TFB_CHANNEL="name=readout-proxy,type=pull,method=connect,address=ipc://@tf-builder-pipe-0,transport=shmem,rateLogging=10"

## TOF-EPN-LOCAL
SHMSIZE=$(( 64 << 30 )) # 64 GiB
SEVERITY="info"
GRP_PATH="--configKeyValues NameConf.mDirGRP=/home/epn/odc/files;NameConf.mDirGeom=/home/epn/odc/files"
ARGS_ALL="--session default --severity $SEVERITY --shm-segment-size $SHMSIZE -b"

CTF_DICT="--ctf-dict /home/epn/odc/files/ctf_dictionary.root"

PROXY_INSPEC="x:TOF/CRAWDATA;dd:FLP/DISTSUBTIMEFRAME/0"
NTHREADS=1

# Output directory for the CTF, to write to the current dir., remove `--output-dir  $CTFOUT` from o2-ctf-writer-workflow or set to `CTFOUT=\"""\"`
# The directory must exist
CTFOUT="/home/epn/odc/tofscripts/debug/out"

o2-dpl-raw-proxy ${ARGS_ALL} --dataspec "${PROXY_INSPEC}" --channel-config "${TFB_CHANNEL}" \
| o2-tof-reco-workflow --input-type raw --output-type clusters,ctf ${ARGS_ALL} ${CTF_DICT} ${GRP_PATH} --disable-root-output --calib-cluster --cluster-time-window 5000 --cosmics \
--pipeline "tof-compressed-decoder:${NTHREADS},TOFClusterer:${NTHREADS},tof-entropy-encoder:${NTHREADS}" \
| o2-tof-cluster-calib-workflow ${ARGS_ALL} --cosmics --output-dir  $CTFOUT \
| o2-ctf-writer-workflow ${ARGS_ALL} ${GRP_PATH} --onlyDet TOF  --output-dir  $CTFOUT  \
| o2-qc ${ARGS_ALL} --config json://${PWD}/qc-full.json --local --host epn \
| o2-dpl-run ${ARGS_ALL} --dds # option instead iof run to export DDS xml file

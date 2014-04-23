#!/bin/bash

LINK_DELAY=`seq 0 100 500`
LOSS_RATIO=`seq 0.1 0.05 0.2`
QPS="1 10 50 100"
ZONES=("10" "50" "100" "245" "500" "1000")
CLIENT=("21" "80" "119" "249" "378" "582")
OUTPUT="output/`date \"+%y%m%d_%H%M\"`"

# Response time measurement
# for DNSSEC

for idx in {0..5}
do
rm -f *.pcap
rm -rf files-*
mkdir -p ${OUTPUT}/dnssec
./waf --run "dce-dnssec-qlog-createzones --processDelay=1 --pcap=0 --disableDnssec=0 --querylog=qlog-${ZONES[$idx]}.log" --dlm 2>&1
cat files-${CLIENT[$idx]}/var/log/*/stdout > ${OUTPUT}/dnssec/stdout-${ZONES[$idx]}.log

# for DNS
rm -f *.pcap
rm -rf files-*
mkdir -p ${OUTPUT}/dns
./waf --run "dce-dnssec-qlog-createzones --processDelay=1 --pcap=0 --disableDnssec=1 --querylog=qlog-${ZONES[$idx]}.log" --dlm 2>&1
cat files-${CLIENT[$idx]}/var/log/*/stdout > ${OUTPUT}/dns/stdout-${ZONES[$idx]}.log

done

chown -R tazaki.tazaki ${OUTPUT}
`dirname ${BASH_SOURCE:-$0}`/dnssec-qlog-plot.sh ${OUTPUT}

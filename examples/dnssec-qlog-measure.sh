#!/bin/bash

LINK_DELAY=`seq 0 100 500`
LOSS_RATIO=`seq 0.1 0.05 0.2`
QPS="1 10 50 100"
CLIENT="1 10 20 30"
OUTPUT="output"

# Response time measurement
# for DNSSEC
rm -f *.pcap
rm -rf files-*
mkdir -p ${OUTPUT}/dnssec
./waf --run "dce-dnssec-qlog-createzones --processDelay=1 --pcap=0 --disableDnssec=0 --querylog=qlog-245.log" 2>&1
cat files-249/var/log/*/stdout > ${OUTPUT}/dnssec/stdout.log

# for DNS
rm -f *.pcap
rm -rf files-*
mkdir -p ${OUTPUT}/dns
./waf --run "dce-dnssec-qlog-createzones --processDelay=1 --pcap=0 --disableDnssec=1 --querylog=qlog-245.log" 2>&1
cat files-249/var/log/*/stdout > ${OUTPUT}/dns/stdout.log


`dirname ${BASH_SOURCE:-$0}`/dnssec-qlog-plot.sh

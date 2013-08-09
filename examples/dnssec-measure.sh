#!/bin/sh

LINK_DELAY=`seq 0 100 500`
LOSS_RATIO=`seq 0.1 0.05 0.2`
QPS="1 10 50 100"
CLIENT="1 10 20 30"

# # LinkDelay (RTT) vs DataBytes measurement
# echo "# link_delay bytes" > rtt-bytes.dat
# echo "# link_delay bytes" > rtt-bytes-dns.dat
# for link in ${LINK_DELAY}; do
# #echo "${link}ms"
# bytes=`./waf --run "dce-dnssec --linkDelay=${link}ms" 2>&1 | grep "Total message" | awk '{print $6}'`
# echo "${link} ${bytes}" >> rtt-bytes.dat
# 
# bytes=`./waf --run "dce-dnssec --linkDelay=${link}ms --disableDnssec=1" 2>&1 | grep "Total message" | awk '{print $6}'`
# echo "${link} ${bytes}" >> rtt-bytes-dns.dat
# done
# 
# # PacketLoss vs DataBytes measurement
# echo "# loss_ratio bytes" > loss-bytes.dat
# echo "# loss_ratio bytes" > loss-bytes-dns.dat
# for loss in ${LOSS_RATIO}; do
# bytes=`./waf --run "dce-dnssec --lossRatio=${loss}" 2>&1 | grep "Total message" | awk '{print $6}'`
# echo "${loss} ${bytes}" >> loss-bytes.dat
# 
# bytes=`./waf --run "dce-dnssec --lossRatio=${loss} --disableDnssec=1" 2>&1 | grep "Total message" | awk '{print $6}'`
# echo "${loss} ${bytes}" >> loss-bytes-dns.dat
# done
# 
# # QPS vs DataBytes measurement
# echo "# qps bytes" > qps-bytes.dat
# echo "# qps bytes" > qps-bytes-dns.dat
# for qps in ${QPS}; do
# bytes=`./waf --run "dce-dnssec --qps=${qps}" 2>&1 | grep "Total message" | awk '{print $6}'`
# echo "${qps} ${bytes}" >> qps-bytes.dat
# 
# bytes=`./waf --run "dce-dnssec --qps=${qps} --disableDnssec=1" 2>&1 | grep "Total message" | awk '{print $6}'`
# echo "${qps} ${bytes}" >> qps-bytes-dns.dat
# done

# Response time measurement
# for DNSSEC
rm -f *.pcap
./waf --run "dce-dnssec --pcap=1 --disableDnssec=0" 2>&1
tcpdump -r dce-dnssec-4-0.pcap -n port domain and host 10.0.0.6 -w test.pcap
tshark -r test.pcap udp.port==53 -t d | grep response | awk '{print $2}' > resp-time-dnssec.dat
# for DNS
rm -f *.pcap
./waf --run "dce-dnssec --pcap=1 --disableDnssec=1" 2>&1
tcpdump -r dce-dnssec-4-0.pcap -n port domain and host 10.0.0.6 -w test.pcap
tshark -r test.pcap udp.port==53 -t d | grep response | awk '{print $2}' > resp-time-dns.dat


#for client in ${CLIENT}; do
#done


gnuplot  << EndGNUPLOT
set ylabel "Total messages (bytes)"
set terminal postscript eps lw 3 "Helvetica" 24
set output "rtt-bytes.eps"
set xrange [0:]
set xtics font "Helvetica,14"
set pointsize 2
set xzeroaxis
set grid ytics

plot \
        'rtt-bytes.dat' w lp title "DNSSEC",  \
        'rtt-bytes-dns.dat' w lp title "DNS"

# set terminal png lw 3 16
# set xtics nomirror rotate by -45 font ",16"
# set output "rtt-bytes.png"
# replot

quit
EndGNUPLOT

gnuplot  << EndGNUPLOT
set ylabel "Total messages (bytes)"
set terminal postscript eps lw 3 "Helvetica" 24
set output "loss-bytes.eps"
set xrange [0:]
set xtics font "Helvetica,14"
set pointsize 2
set xzeroaxis
set grid ytics

plot \
        'loss-bytes.dat' w lp title "DNSSEC", \
        'loss-bytes-dns.dat' w lp title "DNS"

# set terminal png lw 3 16
# set xtics nomirror rotate by -45 font ",16"
# set output "rtt-bytes.png"
# replot

quit
EndGNUPLOT

gnuplot  << EndGNUPLOT
set ylabel "Total messages (bytes)"
set terminal postscript eps lw 3 "Helvetica" 24
set output "qps-bytes.eps"
set xrange [0:]
set xtics font "Helvetica,14"
set pointsize 2
set xzeroaxis
set grid ytics

plot \
        'qps-bytes.dat' w lp title "DNSSEC", \
        'qps-bytes-dns.dat' w lp title "DNS"

# set terminal png lw 3 16
# set xtics nomirror rotate by -45 font ",16"
# set output "rtt-bytes.png"
# replot

quit
EndGNUPLOT

gnuplot  << EndGNUPLOT
set ylabel "Response Time (sec)"
set terminal postscript eps lw 3 "Helvetica" 24
set output "resp-time.eps"
set yrange [0:]
set xtics font "Helvetica,14"
set pointsize 2
set xzeroaxis
set grid ytics

plot \
        'resp-time-dnssec.dat' w p title "DNSSEC", \
        'resp-time-dns.dat' w p title "DNS"

# set terminal png lw 3 16
# set xtics nomirror rotate by -45 font ",16"
# set output "rtt-bytes.png"
# replot

quit
EndGNUPLOT

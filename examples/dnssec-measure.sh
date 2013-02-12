#!/bin/sh

LINK_DELAY=`seq 0 100 500`
LOSS_RATIO=`seq 0.1 0.05 0.2`
QPS="1 10 50 100"
CLIENT="1 10 20 30"

echo "# link_delay bytes" > rtt-bytes.dat
for link in ${LINK_DELAY}; do
#echo "${link}ms"
bytes=`./waf --run "dce-dnssec --linkDelay=${link}ms" 2>&1 | grep "Total message" | awk '{print $6}'`
echo "${link} ${bytes}" >> rtt-bytes.dat
done

echo "# loss_ratio bytes" > loss-bytes.dat
for loss in ${LOSS_RATIO}; do
bytes=`./waf --run "dce-dnssec --lossRatio=${loss}" 2>&1 | grep "Total message" | awk '{print $6}'`
echo "${loss} ${bytes}" >> loss-bytes.dat
done

echo "# qps bytes" > qps-bytes.dat
for qps in ${QPS}; do
bytes=`./waf --run "dce-dnssec --qps=${qps}" 2>&1 | grep "Total message" | awk '{print $6}'`
echo "${qps} ${bytes}" >> qps-bytes.dat
done

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
        'rtt-bytes.dat' w lp

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
        'loss-bytes.dat' w lp

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
        'qps-bytes.dat' w lp

# set terminal png lw 3 16
# set xtics nomirror rotate by -45 font ",16"
# set output "rtt-bytes.png"
# replot

quit
EndGNUPLOT

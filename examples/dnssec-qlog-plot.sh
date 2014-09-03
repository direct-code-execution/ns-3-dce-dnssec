#!/bin/bash

OUTPUT=$1
ZONES=("10" "50" "100" "245" "500" "1000")
NUMZ=("20" "79" "118" "248" "377" "581")


twopi -Groot="." -Teps topology.dot > ${OUTPUT}/topology.eps

for idx in {0..5}
do
grep "Query time" ${OUTPUT}/dnssec/stdout-${ZONES[$idx]}.log  | awk '{print $4/1000}' >  ${OUTPUT}/resp-time-dnssec-${ZONES[$idx]}.dat
grep "Query time" ${OUTPUT}/dns/stdout-${ZONES[$idx]}.log  | awk '{print $4/1000}' >  ${OUTPUT}/resp-time-dns-${ZONES[$idx]}.dat
done

chown -R tazaki.tazaki ${OUTPUT}

#R --no-save <bwplot-qlog.R

R --no-save  << EndR
#R -q --no-save  << EndR
require(lattice)

postscript("${OUTPUT}/resp-time-boxw.eps", horizontal = FALSE, onefile = FALSE, 
           paper = "special", height = 8.9, width = 12.7, family="Helvetica")
par(mar=c(5, 6, 4, 2) + 0.1)

DATA1 <- matrix(rbind(20, scan('${OUTPUT}/resp-time-dnssec-10.dat', comment.char="#"), scan('${OUTPUT}/resp-time-dns-10.dat', comment.char="#")),ncol=3,byrow=T)
DATA2 <- matrix(rbind(79, scan('${OUTPUT}/resp-time-dnssec-50.dat', comment.char="#"), scan('${OUTPUT}/resp-time-dns-50.dat', comment.char="#")),ncol=3,byrow=T)
DATA3 <- matrix(rbind(118, scan('${OUTPUT}/resp-time-dnssec-100.dat', comment.char="#"), scan('${OUTPUT}/resp-time-dns-100.dat', comment.char="#")),ncol=3,byrow=T)
DATA4 <- matrix(rbind(248, scan('${OUTPUT}/resp-time-dnssec-245.dat', comment.char="#"), scan('${OUTPUT}/resp-time-dns-245.dat', comment.char="#")),ncol=3,byrow=T)
DATA5 <- matrix(rbind(377, scan('${OUTPUT}/resp-time-dnssec-500.dat', comment.char="#"), scan('${OUTPUT}/resp-time-dns-500.dat', comment.char="#")),ncol=3,byrow=T)
DATA6 <- matrix(rbind(581, scan('${OUTPUT}/resp-time-dnssec-1000.dat', comment.char="#"), scan('${OUTPUT}/resp-time-dns-1000.dat', comment.char="#")),ncol=3,byrow=T)


D <- rbind (DATA1, DATA2, DATA3, DATA4, DATA5, DATA6)
D <- data.frame(D)
D\$X1 <- factor(D\$X1)

options(scipen=5)
rb <- boxplot (X2 ~ X1, data=D,
        name=c("DNSSEC"),
   	xlab="Number of Zones (n)", ylab="Response Time (sec)",
        col="red", log="y",
	boxwex=0.3, cex.axis=2.5, cex.lab=2.5, lwd=3,lty=1, at=1:6-0.25,
        xlim=c(0.5,6.5), ylim=c(0.001,10))
        #main="outgoing-range=50240",

rb <- boxplot (X3 ~ X1, data=D,
        name=c("DNS"), log="y",
	boxwex=0.3, cex.axis=2.5, cex.lab=2.5, lwd=3,lty=1, at=1:6+0.25,add=T)


legend("topleft", legend = c("DNSSEC", "DNS"), col = c("red", "black"), pch = 18, cex=1.5)
box(lwd=4)


EndR

evince ${OUTPUT}/resp-time-boxw.eps &

exit

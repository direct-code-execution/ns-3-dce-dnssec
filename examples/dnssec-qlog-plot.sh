#!/bin/bash

OUTPUT="output"

grep "Query time" ${OUTPUT}/dnssec/stdout.log  | awk '{print $4/1000}' >  resp-time-dnssec.dat
grep "Query time" ${OUTPUT}/dns/stdout.log  | awk '{print $4/1000}' >  resp-time-dns.dat

#R --no-save <bwplot-qlog.R

R -q --no-save  << EndR
require(lattice)

postscript("resp-time-boxw.eps", horizontal = FALSE, onefile = FALSE, 
           paper = "special", height = 8.9, width = 12.7, family="Helvetica")


DATA1 <- scan("resp-time-dnssec.dat", comment.char="#")
DATA2 <- scan("resp-time-dns.dat", comment.char="#")

#par(tcl=0.4)
par(mar=c(5, 6, 4, 2))
#par(xaxs="i")
#par(yaxs="i")

#boxplot (DATA1, DATA2, names=c("DNSSEC", "DNS"))

rb <- boxplot (DATA1, DATA2, range=0, notch=F, outline=F,
      	names = c("DNSSEC", "DNS"),
   	xlab="", ylab="Response Time (sec)",
	boxwex=0.4, cex.axis=2.5, cex.lab=3, lwd=4,lty=1)
box(lwd=8)


# mn.t <- tapply(D$X2, D$X1, mean)
# sd.t <- tapply(D$X2, D$X1, sd)
# xi <- 0.3 + seq(rb$n)
#points(xi, mn.t, col = "black", pch = 18, cex=2, lwd=4)
#arrows(xi, mn.t - sd.t, xi, mn.t + sd.t,
#       code = 3, col = "black", angle = 75, length = .05, 
#	lty=5, ltw=6)


EndR

evince resp-time-boxw.eps

exit

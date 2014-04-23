#!/bin/bash

OUTPUT=$1
ZONES=("10" "50" "100" "245" "500" "1000")


for idx in {0..5}
do
grep "Query time" ${OUTPUT}/dnssec/stdout-${ZONES[$idx]}.log  | awk '{print $4/1000}' >  ${OUTPUT}/resp-time-dnssec-${ZONES[$idx]}.dat
grep "Query time" ${OUTPUT}/dns/stdout-${ZONES[$idx]}.log  | awk '{print $4/1000}' >  ${OUTPUT}/resp-time-dns-${ZONES[$idx]}.dat
done

chown -R tazaki.tazaki ${OUTPUT}

#R --no-save <bwplot-qlog.R

R -q --no-save  << EndR
require(lattice)

postscript("${OUTPUT}/resp-time-boxw.eps", horizontal = FALSE, onefile = FALSE, 
           paper = "special", height = 8.9, width = 12.7, family="Helvetica")

DATA1 <- matrix(rbind(10, scan('${OUTPUT}/resp-time-dnssec-10.dat', comment.char="#"), scan('${OUTPUT}/resp-time-dns-10.dat', comment.char="#")),ncol=3,byrow=T)
DATA2 <- matrix(rbind(50, scan('${OUTPUT}/resp-time-dnssec-50.dat', comment.char="#"), scan('${OUTPUT}/resp-time-dns-50.dat', comment.char="#")),ncol=3,byrow=T)
DATA3 <- matrix(rbind(100, scan('${OUTPUT}/resp-time-dnssec-100.dat', comment.char="#"), scan('${OUTPUT}/resp-time-dns-100.dat', comment.char="#")),ncol=3,byrow=T)
DATA4 <- matrix(rbind(245, scan('${OUTPUT}/resp-time-dnssec-245.dat', comment.char="#"), scan('${OUTPUT}/resp-time-dns-245.dat', comment.char="#")),ncol=3,byrow=T)
DATA5 <- matrix(rbind(500, scan('${OUTPUT}/resp-time-dnssec-500.dat', comment.char="#"), scan('${OUTPUT}/resp-time-dns-500.dat', comment.char="#")),ncol=3,byrow=T)
DATA6 <- matrix(rbind(1000, scan('${OUTPUT}/resp-time-dnssec-1000.dat', comment.char="#"), scan('${OUTPUT}/resp-time-dns-1000.dat', comment.char="#")),ncol=3,byrow=T)


D <- rbind (DATA1, DATA2, DATA3, DATA4, DATA5, DATA6)
D <- data.frame(D)
D\$X1 <- factor(D\$X1)



rb <- boxplot (X2 ~ X1, data=D, range=0, notch=F, outline=F,
        name=c("DNSSEC"),
   	xlab="Number of Queries (n)", ylab="Response Time (sec)",
        col="red",
	boxwex=0.4, cex.axis=2.5, cex.lab=3, lwd=4,lty=1, at=1:6-0.25)
        #main="outgoing-range=50240",

rb <- boxplot (X3 ~ X1, data=D, range=0, notch=F, outline=F,
        name=c("DNS"),
	boxwex=0.4, cex.axis=2.5, cex.lab=3, lwd=4,lty=1, at=1:6+0.25,add=T)

legend("topleft", legend = c("DNSSEC", "DNS"), col = c("red", "black"), pch = 15)
box(lwd=8)


# mn.t <- tapply(D$X2, D$X1, mean)
# sd.t <- tapply(D$X2, D$X1, sd)
# xi <- 0.3 + seq(rb$n)
#points(xi, mn.t, col = "black", pch = 18, cex=2, lwd=4)
#arrows(xi, mn.t - sd.t, xi, mn.t + sd.t,
#       code = 3, col = "black", angle = 75, length = .05, 
#	lty=5, ltw=6)


EndR

evince ${OUTPUT}/resp-time-boxw.eps

exit

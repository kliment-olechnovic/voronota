#!/bin/bash

INFILE=$1
OUTFILE=$2

R --vanilla --args "$INFILE" "$OUTFILE" << 'EOF'
args=commandArgs(TRUE);
infile=args[1];
outfile=args[2];
t=read.table(infile, header=FALSE, stringsAsFactors=FALSE);
x=t$V1;
y=t$V2;
z=t$V3;
z_max=max(z);
z_min=min(z);
g=(z-z_min)/(z_max-z_min);
r=1-g;
#col=rgb(r*(1/pmax(r, g)), g*(1/pmax(r, g)), 0);
col=rgb(r, g, 0);
png(filename=outfile, width=5, height=5, units="in", res=200);
plot(x=x, y=y, xlab="Position from the start", ylab="Position from the end", col=col, pch=16, cex=2);
dev.off();
EOF

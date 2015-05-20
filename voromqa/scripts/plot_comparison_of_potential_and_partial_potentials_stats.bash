#!/bin/bash

POTENTIAL_FILE=$1
STATS_FILE=$2
OUTDIR=$3

mkdir -p $OUTDIR

R --vanilla --args $POTENTIAL_FILE $STATS_FILE $OUTDIR/comparison_of_potential_and_partial_potentials_stats.png << 'EOF'

args=commandArgs(TRUE);

a=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
b=read.table(args[2], header=FALSE, stringsAsFactors=FALSE);

t=merge(a, b, by=c("V1", "V2", "V3"));
t=t[which(t$V3!="sep1"),];

x=t$V4.x;
y=t$V4.y;
sds=t$V5;

png(args[3],  height=10, width=10, units="in", res=300);

plot(x, y, type="n");
segments(x, y-sds, x, y+sds, col="red");
points(x, y, col="black");
points(c(-100, 100), c(-100, 100), type="l", col="yellow");
points(c(-100, 100), c(0, 0), type="l", col="yellow");
points(c(0, 0), c(-100, 100), type="l", col="yellow");

dev.off();

EOF

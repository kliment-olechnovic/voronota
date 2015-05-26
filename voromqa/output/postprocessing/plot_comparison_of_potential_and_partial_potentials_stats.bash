#!/bin/bash

POTENTIAL_FILE=$1
STATS_FILE=$2
OUTDIR=$3

mkdir -p $OUTDIR

R --vanilla --args $POTENTIAL_FILE $STATS_FILE $OUTDIR/plot_for_tag_ << 'EOF'

args=commandArgs(TRUE);

a=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
b=read.table(args[2], header=FALSE, stringsAsFactors=FALSE);

t=merge(a, b, by=c("V1", "V2", "V3"));

tags=sort(union(t$V3, t$V3));

for(tag in tags)
{
	st=t[which(t$V3==tag),];
	
	x=st$V4.x;
	y=st$V4.y;
	sds=st$V5;
	
	png(paste(args[3], tag, ".png", sep=""),  height=10, width=10, units="in", res=300);
	
	plot(x, y, type="n", main=tag);
	segments(x, y-sds, x, y+sds, col="red");
	points(x, y, col="black");
	points(c(-100, 100), c(-100, 100), type="l", col="yellow");
	points(c(-100, 100), c(0, 0), type="l", col="yellow");
	points(c(0, 0), c(-100, 100), type="l", col="yellow");
	
	dev.off();
}

EOF

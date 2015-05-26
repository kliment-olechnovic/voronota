#!/bin/bash

POTENTIAL_FILE1=$1
POTENTIAL_FILE2=$2
TAG1=$3
TAG2=$4
OUTDIR=$5

mkdir -p $OUTDIR

R --vanilla --args $POTENTIAL_FILE1 $POTENTIAL_FILE2 $TAG1 $TAG2 $OUTDIR/plot_for_${TAG1}_and_${TAG2}.png << 'EOF'

args=commandArgs(TRUE);

a=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
b=read.table(args[2], header=FALSE, stringsAsFactors=FALSE);
tag1=args[3];
tag2=args[4];

a=a[which(a$V3==tag1),];
b=b[which(b$V3==tag2),];
t=merge(a, b, by=c("V1", "V2"));

x=t$V4.x;
y=t$V4.y;
png(args[5], height=10, width=10, units="in", res=300);
plot(x, y, xlab=tag1, ylab=tag2, main=paste(tag1, " vs ", tag2, sep=""));
points(c(-100, 100), c(-100, 100), type="l", col="yellow");
dev.off();

EOF

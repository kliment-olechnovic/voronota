#!/bin/bash

POTENTIAL_FILE=$1
TAG1=$2
TAG2=$3
OUTDIR=$4

mkdir -p $OUTDIR

R --vanilla --args $POTENTIAL_FILE $TAG1 $TAG2 $OUTDIR/plot_tags_${TAG1}_and_${TAG2}.png << 'EOF'

args=commandArgs(TRUE);

t=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
tag1=args[2];
tag2=args[3];

t1=t[which(t$V3==tag1),];
t2=t[which(t$V3==tag2),];
t=merge(t1, t2, by=c("V1", "V2"));

x=t$V4.x;
y=t$V4.y;
png(args[4], height=10, width=10, units="in", res=300);
plot(x, y, xlab=tag1, ylab=tag2, main=paste(tag1, " vs ", tag2, sep=""));
points(c(-100, 100), c(-100, 100), type="l", col="yellow");
dev.off();

EOF

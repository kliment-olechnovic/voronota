#!/bin/bash

POTENTIAL_FILE1=$1
POTENTIAL_FILE2=$2
TAG1=$3
TAG2=$4
OUTDIR=$5

mkdir -p $OUTDIR

R --vanilla --args $POTENTIAL_FILE1 $POTENTIAL_FILE2 $TAG1 $TAG2 $OUTDIR/plot_for__${TAG1}__and__${TAG2}.png << 'EOF'

args=commandArgs(TRUE);

file1=args[1];
file2=args[2];
tag1=args[3];
tag2=args[4];

t1=read.table(file1, header=FALSE, stringsAsFactors=FALSE);
t2=read.table(file2, header=FALSE, stringsAsFactors=FALSE);

t1=t1[which(t1$V3==tag1),];
t2=t2[which(t2$V3==tag2),];
t=merge(t1, t2, by=c("V1", "V2"));

x=t$V4.x;
y=t$V4.y;
sds=c();
if(is.element("V5", colnames(t))) { sds=t$V5; }

png(args[5], height=10, width=10, units="in", res=300);
plot(x, y, type="n", xlab=tag1, ylab=tag2, main=paste(tag1, " vs ", tag2, sep=""));
if(length(sds)>0) { segments(x, y-sds, x, y+sds, col="red"); }
points(x, y, col="black");
points(c(-100, 100), c(-100, 100), type="l", col="yellow");
points(c(-100, 100), c(0, 0), type="l", col="yellow");
points(c(0, 0), c(-100, 100), type="l", col="yellow");
dev.off();

EOF

#!/bin/bash

INPUT=""
OUTPDF=""
OUTTABLE=""

while getopts "i:p:t:" OPTION
do
	case $OPTION in
	i)
		INPUT=$OPTARG
		;;
	p)
		OUTPDF=$OPTARG
		;;
	t)
		OUTTABLE=$OPTARG
		;;
	?)
		echo "Unrecognized option." 1>&2
		exit 1
		;;
	esac
done

mkdir -p $(dirname $OUTPDF)
mkdir -p $(dirname $OUTTABLE)

R --vanilla --args $INPUT $OUTPDF $OUTTABLE <<'EOF'

args=commandArgs(TRUE);

t=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
names=sort(union(t$V1, t$V1));

pdf(args[2]);

histbreaks=seq(min(t$V2)-0.05, max(t$V2)+0.05, 0.01);
hist(t$V2, breaks=histbreaks, col=rgb(1, 0, 0, 0.5), freq=FALSE, main="All atoms");

means=c();
sds=c();
for(name in names)
{
	st=t[which(t$V1==name),];
	means=c(means, mean(st$V2));
	sds=c(sds, sd(st$V2));
	hist(st$V2, breaks=histbreaks, col=rgb(0, 0, 1, 0.5), freq=FALSE, main=name);
}

mt=data.frame(names=names, means=means, sds=sds);
write.table(mt, args[3], col.names=FALSE, row.names=FALSE, quote=FALSE);

EOF

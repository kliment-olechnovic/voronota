#!/bin/bash

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

INDIR=""
PNAME=""
OUTDIR=""

while getopts "d:n:o:" OPTION
do
	case $OPTION in
	d)
		INDIR=$OPTARG
		;;
	n)
		PNAME=$OPTARG
		;;
	o)
		OUTDIR=$OPTARG
		;;
	?)
		echo "Unrecognized option." 1>&2
		exit 1
		;;
	esac
done

find $INDIR/half -type f -name $PNAME -not -empty | while read FNAME
do
	cat $FNAME | awk '{print $4}' > $TMPDIR/$(basename $(dirname $FNAME))
done

mkdir -p $OUTDIR/$PNAME

paste $TMPDIR/* > $OUTDIR/$PNAME/vectors

R --vanilla --args $INDIR/full/$PNAME $OUTDIR/$PNAME/vectors $OUTDIR/$PNAME/means_and_sds $OUTDIR/$PNAME/means_and_sds.png <<'EOF'

args=commandArgs(TRUE);
p=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
m=as.matrix(read.table(args[2], header=FALSE, stringsAsFactors=FALSE));

N=length(m[,1]);
stopifnot(N==length(p[,1]));

column_count=length(m[1,]);
column_sel=c();
for(i in 1:column_count)
{
	if(sd(m[,i])>0)
	{
		column_sel=c(column_sel, i);
	}
}

means=c();
sds=c();

for(i in 1:N)
{
	means=c(means, mean(m[i,column_sel]));
	sds=c(sds, sd(m[i,column_sel]));
}

r=data.frame(a=p[,1], b=p[,2], c=p[,3], val=p[,4], mean=means, sd=sds);
write.table(r, args[3], col.names=TRUE, row.names=FALSE, quote=FALSE);

png(args[4],  height=10, width=10, units="in", res=300);
x=p[,4];
y=means;
plot(x, y);
segments(x, y-sds, x, y+sds);
points(c(-100, 100), c(-100, 100), type="l", col="yellow");
points(c(-100, 100), c(0, 0), type="l", col="yellow");
points(c(0, 0), c(-100, 100), type="l", col="yellow");
points(x, y, col="red");
dev.off();

EOF

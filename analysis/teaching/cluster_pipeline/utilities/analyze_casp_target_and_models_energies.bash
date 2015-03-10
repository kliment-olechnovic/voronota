#!/bin/bash

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

TARGETDIR=""
ENERGYNAME=""
OUTDIR=""

while getopts "t:n:o:" OPTION
do
	case $OPTION in
	t)
		TARGETDIR=$OPTARG
		;;
	n)
		ENERGYNAME=$OPTARG
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

if [ -z "$TARGETDIR" ] || [ -z "$ENERGYNAME" ] || [ -z "$OUTDIR" ]
then
	echo "Missing arguments." 1>&2
	exit 1
fi

if [ ! -s $TARGETDIR/target/$ENERGYNAME ]
then
	echo "Invalid arguments." 1>&2
	exit 1
fi

TARGETNAME=$(basename $TARGETDIR)

echo $TARGETNAME

OUTDIR=$OUTDIR/$ENERGYNAME

REFSIZE=$(cat $TARGETDIR/target/$ENERGYNAME | wc -l)
find $TARGETDIR/models -type f -name $ENERGYNAME -not -empty | while read FNAME
do
	if [ "$(cat $FNAME | wc -l)" -eq "$REFSIZE" ]
	then
		cat $FNAME | awk '{print $2}' > $TMPDIR/$(basename $(dirname $FNAME))
	fi
done

mkdir -p $OUTDIR

paste $TMPDIR/* > $OUTDIR/$TARGETNAME"_vectors"

if [ ! -s "${OUTDIR}/${TARGETNAME}_vectors" ]
then
	echo "Invalid data." 1>&2
	exit 1
fi

R --vanilla --args $TARGETDIR/target/$ENERGYNAME $OUTDIR/$TARGETNAME"_vectors" $OUTDIR/$TARGETNAME"_local_energies.png" $OUTDIR/$TARGETNAME"_best_mean_energies" <<'EOF'

args=commandArgs(TRUE);
t=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
m=as.matrix(read.table(args[2], header=FALSE, stringsAsFactors=FALSE));

N=length(m[,1]);
stopifnot(N==length(t[,1]));

minval=min(c(min(t[,2]), min(m)));
maxval=max(c(max(t[,2]), max(m)));

means=c();
for(i in 1:length(m[1,]))
{
	means=c(means, mean(m[, i]));
}
bestmodelsids=order(means)[1];

sel=order(t[,2]);

png(args[3],  height=10, width=10, units="in", res=300);
plot(x=c(1, N), y=c(minval, maxval), type="n", main=paste("target", mean(t[,2]), "model", mean(m[,bestmodelsids[1]])));
for(i in 1:length(m[1,]))
{
	points(m[sel, i], col="gray", type="l");
}
for(i in bestmodelsids)
{
	points(m[sel, i], col="red", type="l");
}
points(t[sel ,2], col="blue", type="l");
dev.off();

write(c(mean(t[,2]), mean(m[,bestmodelsids[1]])), args[4]);

EOF

rm $OUTDIR/$TARGETNAME"_vectors"

#!/bin/bash

readonly BINDIR=$1
readonly OUTPUTDIRBASE=$2
readonly INPUTFILE=$3

#####################################################

readonly INPUTBASENAME=$(basename $INPUTFILE)
readonly OUTPUTDIR=$OUTPUTDIRBASE/$INPUTBASENAME
mkdir -p $OUTPUTDIR

#####################################################

cat $INPUTFILE \
| $BINDIR/voronota get-balls-from-atoms-file --radii-file $BINDIR/radii --annotated \
| $BINDIR/voronota query-balls --drop-altloc-indicators --drop-atom-serials --drop-adjuncts --drop-tags \
| sed 's/A<OXT>/A<O>/g' \
| grep -f $BINDIR/standard_atom_names \
> $OUTPUTDIR/balls

cat $OUTPUTDIR/balls \
| $BINDIR/voronota calculate-contacts --annotated \
> $OUTPUTDIR/contacts

cat $OUTPUTDIR/contacts \
| $BINDIR/voronota query-contacts --match-min-seq-sep 2 \
| awk '{print $1 " " $2 " " $5 " " $3}' \
| $BINDIR/voronota score-contacts-potential \
> $OUTPUTDIR/contacts_summary

#####################################################

$BINDIR/run_hbplus.bash $BINDIR $INPUTFILE > $OUTPUTDIR/hbplus_output

cat $OUTPUTDIR/contacts \
| $BINDIR/voronota query-contacts --set-hbplus-tags $OUTPUTDIR/hbplus_output --inter-residue-hbplus-tags \
> $OUTPUTDIR/contacts_whb

cat $OUTPUTDIR/contacts_whb \
| $BINDIR/voronota query-contacts --match-min-seq-sep 2 \
| awk '{print $1 " " $2 " " $5 " " $3}' \
| $BINDIR/voronota score-contacts-potential \
> $OUTPUTDIR/contacts_whb_summary

#####################################################

echo $INPUTBASENAME $(cat $OUTPUTDIR/balls | wc -l) $(cat $OUTPUTDIR/contacts | wc -l) > $OUTPUTDIR/size_summary
